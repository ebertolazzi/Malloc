/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2022                                                      |
 |                                                                          |
 |         , __                 , __                                        |
 |        /|/  \               /|/  \                                       |
 |         | __/ _   ,_         | __/ _   ,_                                | 
 |         |   \|/  /  |  |   | |   \|/  /  |  |   |                        |
 |         |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/                       |
 |                           /|                   /|                        |
 |                           \|                   \|                        |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Industriale                              |
 |      Universita` degli Studi di Trento                                   |
 |      email: enrico.bertolazzi@unitn.it                                   |
 |                                                                          |
\*--------------------------------------------------------------------------*/

#include "Utils_Poly.hh"

using namespace std;
using Utils::Poly;

int
main() {

  Poly<double> P( 9 );

  P <<
    0, 0, 1.311999999999999e+15,
    -9.599999999999992e+16,
     2.719999999999998e+18,
    -4.479999999999996e+19,
     4.479999999999996e+20,
    -2.559999999999997e+21,
     6.399999999999994e+21;

  Utils::Sturm<double> S;
  P.normalize();
  S.build( P );

  Utils::Sturm<double>::Integer n_roots = S.separate_roots( 0, 0.1 );
  S.refine_roots(1e-18);
  fmt::print( "Sturm sequence\n{}\n", S );

  //x_roots = P.real_roots_in_the_interval( 0, ell, 1e-20 )

  fmt::print( "ROOTS = {}\n", S.roots().transpose() );

  for ( auto & x : S.roots() ) {
    fmt::print( "P({}) = {}\n", x, P.eval(x) );
  }

  return 0;
}