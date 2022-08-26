% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
function setup( self, max_object_per_node, long_bbox_tolerance, volume_tolerance )
  % bound population count
  if max_object_per_node <= 0
    error('max_object_per_node must be > 0, found %g', max_object_per_node );
  end
  % bound "long" tolerance
  if long_bbox_tolerance < 0 || long_bbox_tolerance > 1
    error('long_bbox_tolerance must be in (0,1) found %g', long_bbox_tolerance );
  end
  % bound "volume_tolerance" tolerance
  if volume_tolerance < 0 || volume_tolerance > 1
    error('volume_tolerance must be in (0,1) found %g', volume_tolerance );
  end
  self.max_object_per_node = max_object_per_node;
  self.long_bbox_tolerance = long_bbox_tolerance;
  self.volume_tolerance    = volume_tolerance;
end
