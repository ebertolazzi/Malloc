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

  // ---------------------------------------------------------------

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
  fmt::print( "N.roots = {}\n", n_roots );

  S.refine_roots(1e-18);
  fmt::print( "Sturm sequence\n{}\n", S );

  //x_roots = P.real_roots_in_the_interval( 0, ell, 1e-20 )

  fmt::print( "ROOTS = {}\n", S.roots().transpose() );

  for ( auto & x : S.roots() ) {
    fmt::print( "P({}) = {}\n", x, P.eval(x) );
  }

  // ---------------------------------------------------------------

  P.set_order(9);
  P <<
    0, 0,
    88740968089804.640625,
    -4915676989941691.0,
    127526512469551888.0,
    -1920443367344816640.0,
    17554326940994666496.0,
    -91691443933113974784.0,
    2.0953255012137562931e+20;

  P.normalize();
  S.build( P );

  n_roots = S.separate_roots( 0, 0.1094 );
  S.refine_roots(1e-18);
  fmt::print( "Sturm sequence\n{}\n", S );

  //x_roots = P.real_roots_in_the_interval( 0, ell, 1e-20 )

  fmt::print( "ROOTS = {}\n", S.roots().transpose() );

  for ( auto & x : S.roots() ) {
    fmt::print( "P({}) = {}\n", x, P.eval(x) );
  }

  // ---------------------------------------------------------------

  P.set_order(9);
  P(0) = 0;
  P(1) = 0;
  P(2) = 87.4724022594292e12+46.8750000000000e-3;
  P(3) = -4.78533691263029e15+2.00000000000000;
  P(4) = 123.408382292671e15+256.000000000000;
  P(5) = -1.84830637948408e18-1.02400000000000e3;
  P(6) = 16.8027852680371e18+6.14400000000000e3;
  P(7) = -87.2871961975953e18-49.1520000000000e3;
  P(8) = 198.379991358171e18+262.144000000000e3;

  P.normalize();
  S.build( P );

  n_roots = S.separate_roots( 0, 0.11 );
  S.refine_roots(1e-18);
  fmt::print( "Sturm sequence\n{}\n", S );

  //x_roots = P.real_roots_in_the_interval( 0, ell, 1e-20 )

  fmt::print( "ROOTS = {}\n", S.roots().transpose() );

  for ( auto & x : S.roots() ) {
    fmt::print( "P({}) = {}\n", x, P.eval(x) );
  }

  P.set_order(9);
  P << 0, 0, -0.005134583085657, 0.211003985866756,-1.363267351305853, 0, 0, 0, 0;

  P.normalize();
  S.build( P );

  n_roots = S.separate_roots( 0, 0.15 );
  S.refine_roots(1e-18);
  fmt::print( "Sturm sequence\n{}\n", S );

  //x_roots = P.real_roots_in_the_interval( 0, ell, 1e-20 )

  fmt::print( "ROOTS = {}\n", S.roots().transpose() );

  for ( auto & x : S.roots() ) {
    fmt::print( "P({}) = {}\n", x, P.eval(x) );
  }


  return 0;
}
