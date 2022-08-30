function info( self )
  nleaf = sum(self.child == 0);
  nlong = sum(self.num_nodes > 0) - nleaf;
  fprintf('AABB tree info-----------------\n');
  fprintf('Dimension           %d\n',size(self.bb_min,2));
  fprintf('Number of nodes     %d\n',length(self.father));
  fprintf('Number of leaf      %d\n',nleaf);
  fprintf('Number of long node %d\n',nlong);
  fprintf('Number of objects   %d\n',length(self.id_nodes));
  fprintf('max_object_per_node %d\n',self.max_object_per_node);
  fprintf('long_bbox_tolerance %g\n',self.long_bbox_tolerance);
  fprintf('volume_tolerance    %g\n',self.volume_tolerance);
  fprintf('-------------------------------\n');
end
