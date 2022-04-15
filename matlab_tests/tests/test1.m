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

% check constructors

A = TestClass();
A.set_a(12);
A.set_b(1);
A.set_c(32);
disp('A');
A.info;
B = copy(A);
C = A;
B.set_a(100);
C.set_a(101);

disp('A');
A.info;

disp('B');
B.info;

disp('C');
C.info;

%A.delete
A = TestClass();
B.set_b(1233);

disp('A deleted --> C deleted');
C.set_b(1233);

disp('C');
C.info;

C = A;
A.delete;

disp('A deleted --> C deleted');
C.set_b(1233);

