% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
function plot( self )
  % - - - - - - - - - - - - - - - - - - - - - - -  find non empty nodes
  lf = find(self.num_nodes>0);
  %----------------------------------------------- tree in R^p
  np  = numel(find(lf));
  idx = (1:np)';
  %------------------------------------------------- nodes
  min_x = self.bb_min(lf,1);
  min_y = self.bb_min(lf,2);
  max_x = self.bb_max(lf,1);
  max_y = self.bb_max(lf,2);

  fc = [.95,.95,.55];
  ec = [.15,.15,.15];

  %-------------------------- draw all "leaf" nodes as patches
  switch(size(self.bb_min,2))
  case 2
    pp = [
      min_x, min_y,
      max_x, min_y,
      max_x, max_y,
      min_x, max_y
    ];
    %------------------------------------------------- faces
    bb  = [ idx, idx+np, idx+2*np, idx+3*np ];

  case 3
    min_z = self.bb_min(lf,3);
    max_z = self.bb_max(lf,3);
    pp = [
      min_x, min_y, min_z,
      max_x, min_y, min_z,
      max_x, max_y, min_z,
      min_x, max_y, min_z,
      min_x, min_y, max_z,
      max_x, min_y, max_z,
      max_x, max_y, max_z,
      min_x, max_y, max_z
    ];
    %------------------------------------------------- faces
    bb  = [
      idx,      idx+np,   idx+2*np, idx+3*np,
      idx+4*np, idx+5*np, idx+6*np, idx+7*np,
      idx,      idx+3*np, idx+7*np, idx+4*np,
      idx+3*np, idx+2*np, idx+6*np, idx+7*np,
      idx+2*np, idx+np,   idx+5*np, idx+6*np,
      idx+np,   idx,      idx+4*np, idx+5*np
    ];

  otherwise
    %--------------------------- what to do with a tree in R^d!?
    error('scantree:unsupportedDimension Unsupported d-dimensionality.') ;
  end

  patch( ...
    'faces',     bb, ...
    'vertices',  pp, ...
    'facecolor', fc, ...
    'edgecolor', ec, ...
    'facealpha', 0.2 ...
  );
end
