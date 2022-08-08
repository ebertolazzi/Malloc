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

NN = 150;

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
  SEGS2{k} = Segment(Pa,Pb);
end

subplot(1,2,1);
hold on;
bb_max1 = zeros(NN,2);
bb_min1 = zeros(NN,2);
for k=1:NN
  SEGS1{k}.plot( '-k', 'LineWidth', 2 );
  B1 = SEGS1{k}.bbox(k);
  [ma,mi] = B1.get_max_min();
  bb_min1(k,:) = mi.';
  bb_max1(k,:) = ma.';
end

nobj = 1;
long = 0.8;
vtol = 0.55;

tr1 = AABBtree(nobj,long,vtol);
tr1.build(bb_min1,bb_max1);
tr1.plot();
axis equal

subplot(1,2,2);
hold on;
bb_max2 = zeros(NN,2);
bb_min2 = zeros(NN,2);
for k=1:NN
  SEGS2{k}.plot( '-r', 'LineWidth', 2 );
  B2 = SEGS2{k}.bbox(k);
  [ma,mi] = B2.get_max_min();
  bb_min2(k,:) = mi.';
  bb_max2(k,:) = ma.';
end

tr2 = AABBtree(nobj,long,vtol);
tr2.build(bb_min2,bb_max2);
tr2.plot();
axis equal
