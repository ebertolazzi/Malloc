%=========================================================================%
%                                                                         %
%  Autors: Enrico Bertolazzi                                              %
%          Department of Industrial Engineering                           %
%          University of Trento                                           %
%          enrico.bertolazzi@unitn.it                                     %
%                                                                         %
%=========================================================================%

close all;

addpath('../lib');

NN = 100;

% check constructors
fprintf('Generate lines\n');
SEGS1 = {};
for k=1:NN
  if k==1
    x     = 10*rand;
    y     = 10*rand;
    theta = 2*pi*rand;
  else
    x     = mod( Pb(1) + rand/4, 10 );
    y     = mod( Pb(2) + rand/4, 10 );
    theta = theta+pi/10*rand;
  end
  len      = 0.01+2*rand;
  Pa       = [x;y];
  Pb       = Pa + len*[cos(theta);sin(theta)];
  SEGS1{k} = Segment(Pa,Pb);
end

SEGS2 = {};
OX    = 4;
OY    = 3;
for k=1:NN
  if k==1
    x     = OX+10*rand;
    y     = OY+10*rand;
    theta = 2*pi*rand;
  else
    x     = mod( Pb(1) + rand/4 - OX, 10 )+ OX;
    y     = mod( Pb(2) + rand/4 - OY, 10 )+ OY;
    theta = theta+pi/10*rand;
  end
  len      = 0.01+2*rand;
  Pa       = [x;y];
  Pb       = Pa + len*[cos(theta);sin(theta)];
  SEGS2{k} = Segment(Pa,Pb);
end

subplot(1,2,1);
hold on;
bb_max1 = zeros(NN,2);
bb_min1 = zeros(NN,2);
for k=1:NN
  SEGS1{k}.plot( '-r', 'LineWidth', 1 );
  B1 = SEGS1{k}.bbox(k);
  bb_min1(k,:) = B1.get_min().';
  bb_max1(k,:) = B1.get_max().';
end

nobj = 10;
long = 0.8;
vtol = 0.55;

tr1 = AABBtree(nobj,long,vtol);
tr1.build(bb_min1,bb_max1);
tr1.plot('red', 'black');
tr1.info
%xlim([0,20]);
%axis equal

%subplot(2,2,2);
%hold on;
bb_max2 = zeros(NN,2);
bb_min2 = zeros(NN,2);
for k=1:NN
  SEGS2{k}.plot( '-b', 'LineWidth', 1 );
  B2 = SEGS2{k}.bbox(k);
  bb_min2(k,:) = B2.get_min().';
  bb_max2(k,:) = B2.get_max().';
end

tr2 = AABBtree(nobj,long,vtol);
tr2.build(bb_min2,bb_max2);
tr2.plot('blue', 'black');
tr2.info
%xlim([0,15]);
axis equal

%
% trovo candidati intersezioni
%

subplot(1,2,2);

if false

  [b1min,b1max] = tr1.get_bb_min_max();
  [b2min,b2max] = tr2.get_bb_min_max();

  ii = 2;
  ok_list1 = tr1.intersect_with_one_bbox(b2min(ii,:),b2max(ii,:));
  ok_list2 = tr2.intersect_with_one_bbox(b1min(ii,:),b1max(ii,:));

  if false
    tr1.plot_bbox( b1min(ok_list1,:), b1max(ok_list1,:), 'red',  'black' );
    tr1.plot_bbox( b2min(ii,:),       b2max(ii,:),       'blue', 'black' );
  else
    tr2.plot_bbox( b2min(ok_list2,:), b2max(ok_list2,:), 'blue', 'black' );
    tr2.plot_bbox( b1min(ii,:),       b1max(ii,:),       'red',  'black' );
  end
else

  [id_list1,id_list2] = tr1.intersect(tr2);

  [mi1,ma1] = tr1.get_bb_min_max();
  [mi2,ma2] = tr2.get_bb_min_max();
  
  %ok = ~cellfun(@isempty,id_list1);
  %tr1.plot_bbox( mi1(ok,:), ma1(ok,:), 'red', 'black' );

  %ok = ~cellfun(@isempty,id_list2);
  %tr2.plot_bbox( mi2(ok,:), ma2(ok,:), 'blue', 'black' );

  for k=1:length(id_list1)
    idx = id_list1{k};
    if ~isempty(idx)
      tr1.plot_bbox( mi1(k,:),     ma1(k,:),     'red',  'black' );
      %tr2.plot_bbox( b2min(idx,:), b2max(idx,:), 'blue', 'cyan'  );
      tr2.plot_bbox( bb_min2(idx,:), bb_max2(idx,:), 'blue', 'cyan'  );
      %break;
    end
  end

  %ok = ~cellfun(@isempty,id_list2);
  %[mi,ma] = tr2.get_bb_min_max();
  %tr2.plot_bbox( mi(ok,:), ma(ok,:), 'blue', 'black' );

end

hold on;
for k=1:NN
  SEGS1{k}.plot( '-r', 'LineWidth', 1 );
  SEGS2{k}.plot( '-b', 'LineWidth', 1 );
end

%xlim([0,15]);
axis equal

