function test_AABB2
  close all
%-----------------------------------------------------------
    fprintf(1,[...
'   AABBTREE facilitates efficient spatial queries through\n',...
'   "localisation" -- reducing a large O(M*N) comparison  \n',...
'   to a sequence of small O(m*n) operations, where m<<M  \n',...
'   and n<<N. By partitioning the data about the aabb-tree\n',...
'   itself, intersection tests can be carried out over sm-\n',...
'   all local subsets, rather than between every pair of  \n',...
'   objects in the collection. \n\n',...
'   In the following example, the intersections between a \n',...
'   set of points and a set of circles is computed.       \n',...
'   The "slow" algorithm simply tests every point against \n',...
'   every circle (an O(N^2) operation). \n\n',...
'   The "fast" algorithm relies on an aabb-tree to partit-\n',...
'   ion the data, and then computes the intersections loc-\n',...
'   ally (an approx. O(N*log(N) operation)). The speed-up \n',...
'   is around a factor of 10 (on my machine).\n\n',...
'   The QUERYSET routine has been provided to compute such\n',...
'   operations, taking an aabb-tree, a tree-query mapping,\n',...
'   and a user-defined query function as inputs. See the  \n',...
'   DEMO-3 code in AABBDEMO.m for additional details.\n\n']);

  nc = 10000*4;
  np = 50000*4;

  pc = randcirc(nc,2,0.02);
  pi = rand(np,size(pc,2)-1);

  fprintf(1,'   "Slow" algorithm: \n');

  tic;
  [ii_slow,ip_slow,cj_slow   ] = slowfindcirc(pc,pi);
  toc

  fprintf(1,'   "Fast" algorithm: \n');

  start = tic;
  [ii_fast,ip_fast,cj_fast,tr] = fastfindcirc(pc,pi);
  toc(start)

  fprintf(1,'   Equivalent results? \n');

  if size(ii_slow) == size(ii_fast)
    same = true ;
    for ii=1:size(ip_slow,1)
      c1 = cj_slow(ip_slow(ii,1):ip_slow(ii,2));
      c2 = cj_fast(ip_fast(ii,1):ip_fast(ii,2));
      c1 = sort(c1);
      c2 = sort(c2);
      if length(c1) == length(c2)
        if any(c1 ~= c2)
          same = false;
          break;
        end
      else
        same = false;
        break;
      end
    end
  else
    same = false;
  end
  if same
    fprintf(1,'   TRUE \n');
  else
    fprintf(1,'  FALSE \n');
  end

  vp = ceil(linspace(+1,min(2500,np),2500));
  vc = ceil(linspace(+1,min(5000,nc),5000));

  figure;
  subplot(1,2,1); hold on;
  drawcirc(pc(vc,:));
  plot(pi(vp,1),pi(vp,2),'r.');
  axis image off;
  set(gca,'units','normalized','position',[0.01,0.05,.48,.90]);
  subplot(1,2,2); hold on;
  tr.plot();
  axis image off;
  set(gca,'units','normalized','position',[0.51,0.05,.48,.90]);
end



function [ii,ip,cj] = slowfindcirc(pc,pi)
%SLOWFINDCIRC find the points enclosed by a set of circles.
%   [II,IP,CJ] = SLOWFINDCIRC(PC,PI) computes the pairwise
%   point-circle intersections between the points PI and
%   the circles PC. PC(:,1:2) are the circle centres and
%   PC(:,3) are the circle radii.
%   [II,IP,CJ] is the set of intersections in compressed
%   "sparse-style" indexing. Each point II(K) intersects
%   with the list of circles CJ(IP(K,1):IP(K,2)).
%
%   This is the "slow" brute-force variant.

  ip = 1:size(pi,1);
  ic = 1:size(pc,1);

  [pj,cj] = incircle(ip,ic,pi,[pc(:,1:2),pc(:,3).^2]);

  pj = pj( :);
  cj = cj( :);

  %-- re-index to the sparse-style representation of QUERYSET
  [pj,ix] = sort (pj);
  cj = cj(ix);
  ix = find(diff(pj)>0);

  %-- the points in II intersect with >= 1 circle
  ni = length(pj);
  ii = pj([ix;ni]);

  nj = length(cj);
  ni = length(ii);

  %-- the points in II intersect with the circles
  %-- CJ(IP(K,1):IP(K,2)) {for point II(K)}.
  ip      = zeros(ni,2) ;
  ip(:,1) = [+1; ix+1];
  ip(:,2) = [ix; nj+0];
end


function [ii,ip,cj,tr] = fastfindcirc(pc,pi)
%FASTFINDCIRC find the points enclosed by a set of circles.
%   [II,IP,CJ] = FASTFINDCIRC(PC,PI) computes the pairwise
%   point-circle intersections between the points PI and
%   the circles PC. PC(:,1:2) are the circle centres and
%   PC(:,3) are the circle radii.
%   [II,IP,CJ] is the set of intersections in compressed
%   "sparse-style" indexing. Each point II(K) intersects
%   with the list of circles CJ(IP(K,1):IP(K,2)).
%
%   This is the "fast" aabb-indexed variant.

  fprintf('BUILD:     ');
  tic

  nd = size(pc,2)-1;

  %-- compute the set of aabb's for circ.
  bb_min = zeros(size(pc,1),nd);
  bb_max = zeros(size(pc,1),nd);
  for id=1:nd
    bb_min(:,id) = pc(:,id)-pc(:,nd+1);
    bb_max(:,id) = pc(:,id)+pc(:,nd+1);
  end

  %-- compute aabb-tree for circles
  tr = AABBtree();
  tr.build(bb_min,bb_max);
  %tr.info

  %-- compute aabb-tree for points
  tr2 = AABBtree();
  tr2.build(pi,pi);
  %tr2.info

  toc

  fprintf('SCAN:      ');
  tic
  %id_list = tr.scan_points( pi );
  %id_list = tr.scan_bbox( pi, pi );
  id_list = tr.intersect(tr2);
  toc

  fprintf('INTERSECT: ');
  tic
  nn = tr.get_num_nodes();
  pj = cell(nn,1);
  cj = cell(nn,1);
  for k=1:nn
    if ~isempty(id_list{k})
      [pj{k},cj{k}] = incircle(id_list{k},tr.get_nodes(k),pi,[pc(:,1:2),pc(:,3).^2]);
    end
  end
  pj = vertcat(pj{:});
  cj = vertcat(cj{:});
  toc


  %pj = pj(:);
  %cj = cj(:);

  %-- re-index to the sparse-style representation of QUERYSET
  [pj,ix] = sort (pj);
  cj = cj(ix);
  ix = find(diff(pj)>0);

  %-- the points in II intersect with >= 1 circle
  ni = length(pj);
  ii = pj([ix;ni]);

  nj = length(cj);
  ni = length(ii);

  %-- the points in II intersect with the circles
  %-- CJ(IP(K,1):IP(K,2)) {for point II(K)}.
  ip = zeros(ni,2) ;
  ip(:,1) = [+1; ix+1];
  ip(:,2) = [ix; nj+0];

end

function [pj,cj] = incircle(ip,ic,pi,pc)
%INCIRCLE pairwise point-circle comparison kernel function.
%   [PJ,CJ] = INCIRCLE(IP,IC,PI,PC) compute the pairwise in-
%   tersections between the points PI(IP,:) and the circles
%   PC(IC,:). PC(:,3) are the squared circle radii.
%   [PJ,CJ] are pairs of intersections, such that the point
%   PJ(K) intersects with the circle CJ(K).

  li = cell(length(ic),1);
  lj = cell(length(ic),1);

  pt = pi(ip,:);

  for ii = 1:length(ic)
    di     = (pt(:,1)-pc(ic(ii),1)).^2 + (pt(:,2)-pc(ic(ii),2)).^2 ;
    li{ii} = find(di<=pc(ic(ii),3));
    lj{ii} = ii * ones(length(li{ii}),1);
  end

  pj = ip(vertcat(li{:}));
  cj = ic(vertcat(lj{:}));

end

function [pc] = randcirc(nc,nd,rr)
  %RANDCIRC make a set of NC randomised d-circles in R^ND with
  %mean radius RR.
  pc = [rand(nc,nd), rr*(rand(nc,1))];
end

function drawcirc(pc)
  %DRAWCIRC draw a set of NC d-circles.
  fc = [.95,.95,.55];
  ec = [.25,.25,.25];
  switch (size(pc,2))
  case 3
    %--------------------------------------------------- circles
    tt = linspace(0,2*pi,24);
    xx = cos(tt)';
    yy = sin(tt)';
    nt = length(tt);

    ee = [(1:nt-1)',(2:nt-0)'];

    xc = cell(size(pc,1),1);
    yc = cell(size(pc,1),1);
    jc = cell(size(pc,1),1);

    for ic = 1:size(pc,1)

      xc{ic} = pc(ic,3)*xx + pc(ic,1);
      yc{ic} = pc(ic,3)*yy + pc(ic,2);

      jc{ic} = ee + (ic-1)*nt;
    end

    pp = [vertcat(xc{:}), vertcat(yc{:})];
    ff = vertcat(jc{:});
  case 4
    %--------------------------------------------------- spheres
    %%!!todo:
  end

  patch('faces',ff,'vertices',pp,'facecolor',fc,'edgecolor',ec,'facealpha',+.3);

end
