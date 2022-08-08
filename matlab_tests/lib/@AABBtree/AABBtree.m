%MAKETREE assemble an AABB search tree for a collection of
%(hyper-)rectangles.
%   [TR] = MAKETREE(RP) returns an axis-aligned bounding-box
%   (AABB) tree for a collection of d-rectangles embedded in
%   R^d. The rectangles are defined as an NR-by-NDIM*2 array
%   of min/max coordinates RP = [PMIN,PMAX], where PMIN =
%   [A1,A2,...,ANDIM] and PMAX = [B1,B2,...,BNDIM] are the
%   minimum/maximum coordinates associated with each rectan-
%   gle.
%
%   The resulting tree is returned as a structure containing
%   an NT-by-NDIM*2 array of tree-node coordinates TR.XX =
%   [PMIN,PMAX], an NT-by-2 array of tree-node indexing
%   TR.II = [PI,CI], and an NT-by-1 cell array of node lists
%   TR.LL. PI,CI are the parent/child pointers associated
%   with each node in the tree, and TR.LL{II} is the list of
%   rectangles associated with the II-th node.
%
%   MAKETREE forms a binary search tree, such that each
%   "leaf" node in the tree encloses a maximum number of re-
%   ctangles. The tree is formed by recursively subdividing
%   the bounding-box of the collection. At each division, a
%   simple heuristic is used to determine a splitting axis
%   and to position an axis-aligned splitting (hyper-)plane.
%   The associated collection of rectangles is partitioned
%   between two new child nodes. The dimensions of each node
%   in the tree are selected to provide a minimal enclosure
%   of the rectangles in its associated sub-tree. Tree nodes
%   may overlap as a result.
%
%   [...] = MAKETREE(RP,OP) also passes an additional user-
%   defined options structure. OP.NOBJ = {32} is the maximum
%   allowable number of rectangles per tree-node. OP.LONG =
%   {.75} is a relative length tolerance for "long" rectang-
%   les, such that any rectangles with RMAX(IX)-RMIN(IX) >=
%   OP.LONG * (NMAX(IX)-NMIN(IX)) remain in the parent node.
%   Here RMIN,RMAX are the coordinates of the rectangle,
%   NMIN,NMAX are the coordinates of the enclosing node in
%   the tree, and IX is the splitting axis. Nodes that beco-
%   me "full" of "long" items may exceed their maximum rect-
%   angle capacity. OP.VTOL = {.55} is a "volume" splitting
%   criteria, designed to continue subdivision while the net
%   node volume is reducing. Specifically, a node is split
%   if V1+V2 <= OP.VTOL*VP, where VP is the d-dim. "volume"
%   of the parent node, and V1,V2 are the volumes associated
%   with its children.
%
%   See also DRAWTREE, QUERYSET, MAPVERT, MAPRECT

% Please see the following for additional information:
%
%   Darren Engwirda, "Locally-optimal Delaunay-refinement &
%   optimisation-based mesh generation". Ph.D. Thesis, Scho-
%   ol of Mathematics and Statistics, Univ. of Sydney, 2014:
%   http://hdl.handle.net/2123/13148

%   Darren Engwirda : 2014 --
%   Email           : de2363@columbia.edu
%   Last updated    : 08/10/2017
%
classdef AABBtree < matlab.mixin.Copyable
  properties (SetAccess = private, Hidden = true)
    father;
    child;
    % --------
    ptr_nodes;
    num_nodes;
    id_nodes;
    % --------
    bb_min;
    bb_max;
    % --------
    long;
    vtol;
    NOBJ;
  end
  methods(Access = protected)
    % Override copyElement method:
    function obj = copyElement( self )
      obj           = copyElement@matlab.mixin.Copyable(self);
      obj.father    = self.father;
      obj.child     = self.child;
      obj.ptr_nodes = self.ptr_nodes;
      obj.num_nodes = self.num_nodes;
      obj.id_nodes  = self.id_nodes;
      obj.bb_min    = self.bb_min;
      obj.bb_max    = self.bb_max;
      obj.long      = self.long;
      obj.vtol      = self.vtol;
      obj.NOBJ      = self.NOBJ;
    end
  end
  methods
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function self = AABBtree( varargin )
      isoctave = exist('OCTAVE_VERSION','builtin') > 0;
      if (isoctave)
        %-- faster for OCTAVE with large tree block size; slower loop execution.
        NOBJ = 1024;
      else
        %-- faster for MATLAB with small tree block size; better loop execution.
        NOBJ = 32;
      end
      long = 0.8;
      vtol = 0.1;
      if nargin > 0; NOBJ = varargin{1}; end
      if nargin > 1; long = varargin{2}; end
      if nargin > 2; vtol = varargin{3}; end
      self.setup( NOBJ, long, vtol );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function str = is_type( ~ )
      str = 'AABBtree';
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    setup( self, NOBJ, long, vtol )
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    build( self, bb_min, bb_max )
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function [f,c,ptr,num,id] = get_tree_structure( self )
      f   = self.father;
      c   = self.child;
      ptr = self.ptr_nodes;
      num = self.num_nodes;
      id  = self.id_nodes;
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    inodes = get_nodes( self, id )
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function N = get_num_nodes( self )
      N = length(self.father);
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    id_list = scan_points( self, pnts )
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    id_list = scan_bbox( self, bb_min, bb_max )
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    plot( self )
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  end
end
