function ok_list = intersect_with_one_bbox( self, bb_min, bb_max )
  sz      = length(self.father);
  ok_list = false(sz,1);

  % quick return on empty inputs
  if sz == 0; return; end

  % allocation
  stack = zeros(sz,1);

  % descend tree from root
  root       = 1;
  stack(1)   = root;
  top_stack  = 1;
  bmin       = self.bb_min(root,:);
  bmax       = self.bb_max(root,:);
  if self.bbox_overlap( bb_min, bb_max, bmin, bmax )
    ok_list(1) = true;
  end
  while top_stack > 0
    % pop node from stack
    id_father = stack(top_stack);
    top_stack = top_stack - 1;

    % list of points that may intersect
    if self.num_nodes(id_father) == 0
      ok_list(id_father) = false;
    end

    if self.child(id_father) == 0
      % no children, nothing to do
      continue;
    end

    for kkk=0:1
      id_lr  = self.child(id_father)+kkk;
      lr_min = self.bb_min(id_lr,:);
      lr_max = self.bb_max(id_lr,:);

      % check intersetion
      if self.bbox_overlap( bb_min, bb_max, lr_min, lr_max )
        % push nonempty node onto stack
        top_stack        = top_stack + 1;
        stack(top_stack) = id_lr;
        ok_list(id_lr)   = true;
      end
    end
  end
end
