%-------------------------------------------------------+
%                                                       |
% Copyright (C) 2022                                    |
%                                                       |
%        , __                 , __                      |
%       /|/  \               /|/  \                     |
%        | __/ _   ,_         | __/ _   ,_              |
%        |   \|/  /  |  |   | |   \|/  /  |  |   |      |
%        |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/     |
%                          /|                   /|      |
%                          \|                   \|      |
%                                                       |
%     Enrico Bertolazzi                                 |
%     Dipartimento di Ingegneria Industriale            |
%     Universita` degli Studi di Trento                 |
%     email: enrico.bertolazzi@unitn.it                 |
%                                                       |
%-------------------------------------------------------+

classdef AABB_tree < matlab.mixin.Copyable

  properties (SetAccess = private, Hidden = true)
    objectHandle;
  end
  methods(Access = protected)
    % Override copyElement method:
    function obj = copyElement( self )
      obj = copyElement@matlab.mixin.Copyable(self);
      obj.objectHandle = AABB_treeMexWrapper( 'copy', self.objectHandle );
    end
  end
  methods
    function self = AABB_tree( varargin )
      self.objectHandle = AABB_treeMexWrapper( 'new' );
      if nargin > 0
        self.setup( varargin{:} )
      end
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function str = is_type( ~ )
      str = 'AABB_tree';
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function handle = handle( self )
      handle = self.objectHandle;
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function delete(self)
      AABB_treeMexWrapper( 'delete', self.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function setup( self, max_object_per_node, long_bbox_tolerance, volume_tolerance )
      self.set_max_object_per_node( max_object_per_node );
      self.set_long_bbox_tolerance( long_bbox_tolerance );
      self.set_volume_tolerance( volume_tolerance );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function set_max_object_per_node( self, max_object_per_node )
      AABB_treeMexWrapper( ...
        'set_max_object_per_node', self.objectHandle, max_object_per_node ...
      );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function set_long_bbox_tolerance( self, long_bbox_tolerance )
      AABB_treeMexWrapper( ...
        'set_long_bbox_tolerance', self.objectHandle, long_bbox_tolerance ...
      );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function set_volume_tolerance( self, volume_tolerance )
      AABB_treeMexWrapper( ...
        'set_volume_tolerance', self.objectHandle, volume_tolerance ...
      );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function build( self, bb_min, bb_max, varargin )
      do_transpose = false;
      if nargin > 3; do_transpose = varargin{1}; end
      if do_transpose
        AABB_treeMexWrapper( 'build', self.objectHandle, bb_min.', bb_max.' );
      else
        AABB_treeMexWrapper( 'build', self.objectHandle, bb_min, bb_max );
      end
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function [ bb_min, bb_max ] = get_bboxes_of_the_tree( self, varargin )
      nmin = 0;
      if nargin > 1; nmin = varargin{1}; end
      [bb_min,bb_max] = AABB_treeMexWrapper( 'get_bboxes_of_the_tree', self.objectHandle, nmin );
      if nargin > 2
        if varargin{2}
          bb_min = bb_min.';
          bb_max = bb_max.';
        end
      end
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function id = get_bbox_indexes_of_a_node( self, inode )
      id = AABB_treeMexWrapper( 'get_bbox_indexes_of_a_node', self.objectHandle, inode );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function N = get_num_tree_nodes( self, varargin )
      if nargin > 1
        N = AABB_treeMexWrapper( 'get_num_tree_nodes', self.objectHandle, varargin{1} );
      else
        N = AABB_treeMexWrapper( 'get_num_tree_nodes', self.objectHandle, 0 );
      end
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function id_list = scan_points( self, pnts )
      id_list = AABB_treeMexWrapper( 'scan_points', self.objectHandle, pnts );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function id_list = scan_bbox( self, bb_min, bb_max )
      id_list = AABB_treeMexWrapper( 'scan_bbox', self.objectHandle, bb_min, bb_max );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function id_list = intersect_with_one_bbox( self, bb_min, bb_max )
      id_list = AABB_treeMexWrapper( 'intersect_with_one_bbox', self.objectHandle, bb_min, bb_max );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function id_list = intersect( self, aabb )
      id_list = AABB_treeMexWrapper( 'intersect', self.objectHandle, aabb.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function id_list = intersect_and_refine( self, aabb, bb1_min, bb1_max, bb2_min, bb2_max, varargin )
      do_transpose = false;
      if nargin > 6; do_transpose = varargin{1}; end
      if do_transpose
        id_list = AABB_treeMexWrapper( ...
          'intersect_and_refine', self.objectHandle, ...
          aabb.objectHandle, bb1_min.', bb1_max.', bb2_min.', bb2_max.' ...
        );
      else
        id_list = AABB_treeMexWrapper( ...
          'intersect_and_refine', self.objectHandle, ...
          aabb.objectHandle, bb1_min, bb1_max, bb2_min, bb2_max ...
        );
      end
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function plot( self, varargin )
      % find non empty nodes
      [ bb_min, bb_max ] = self.get_bboxes_of_the_tree( 1 );
      if nargin > 1
        fc = varargin{1};
      else
        fc = [.95,.95,.55];
      end
      if nargin > 2
        ec = varargin{2};
      else
        ec = [.15,.15,.15];
      end
      self.plot_bbox( bb_min.', bb_max.', fc, ec );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function plot_bbox( self, mi, ma, fc, ec )
      np    = size(mi,1);
      idx   = (1:np)';
      min_x = mi(:,1);
      min_y = mi(:,2);
      max_x = ma(:,1);
      max_y = ma(:,2);
      % draw all "leaf" nodes as patches
      switch(size(mi,2))
      case 2
        pp = [ min_x, min_y; max_x, min_y; max_x, max_y; min_x, max_y ];
        % faces
        bb = [ idx, idx+np, idx+2*np, idx+3*np ];
      case 3
        min_z = mi(:,3);
        max_z = ma(:,3);
        pp = [ ...
          min_x, min_y, min_z; ...
          max_x, min_y, min_z; ...
          max_x, max_y, min_z; ...
          min_x, max_y, min_z; ...
          min_x, min_y, max_z; ...
          max_x, min_y, max_z; ...
          max_x, max_y, max_z; ...
          min_x, max_y, max_z  ...
        ];
        % faces
        bb = [
          idx,      idx+np,   idx+2*np, idx+3*np,
          idx+4*np, idx+5*np, idx+6*np, idx+7*np,
          idx,      idx+3*np, idx+7*np, idx+4*np,
          idx+3*np, idx+2*np, idx+6*np, idx+7*np,
          idx+2*np, idx+np,   idx+5*np, idx+6*np,
          idx+np,   idx,      idx+4*np, idx+5*np
        ];
      otherwise
        % what to do with a tree in R^d!?
        error('Unsupported d-dimensionality.') ;
      end
      patch( ...
        'faces',     bb, ...
        'vertices',  pp, ...
        'facecolor', fc, ...
        'edgecolor', ec, ...
        'facealpha', 0.2 ...
      );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function info( self )
      AABB_treeMexWrapper( 'info', self.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  end
end
