/****************************************************************************\
  Copyright (c) Enrico Bertolazzi 2019
  All Rights Reserved.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the file license.txt for more details.
\****************************************************************************/

#include "Utils_mex.hh"
#include "Utils/mex_workaround.hxx"
#include <fstream>

#include "TestClass.hh"

namespace Utils {

  /*\
   *                      _____                 _   _
   *  _ __ ___   _____  _|  ___|   _ _ __   ___| |_(_) ___  _ __
   * | '_ ` _ \ / _ \ \/ / |_ | | | | '_ \ / __| __| |/ _ \| '_ \
   * | | | | | |  __/>  <|  _|| |_| | | | | (__| |_| | (_) | | | |
   * |_| |_| |_|\___/_/\_\_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
   *
  \*/

  static
  void
  do_new(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *[] // unused
  ) {
    #define MEX_ERROR_MESSAGE_1 "TestClassMexWrapper('new')"
    #define CMD MEX_ERROR_MESSAGE_1
    UTILS_MEX_ASSERT( nlhs == 1, "{}: expected 1 output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 1, "{}: expected 1 input, nrhs = {}\n", CMD, nrhs );

    arg_out_0 = Utils::mex_convert_ptr_to_mx<TestClass>(new TestClass());

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_delete(
    int nlhs, mxArray       *[], // unused
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_2 "TestClassMexWrapper('delete',obj)"
    #define CMD MEX_ERROR_MESSAGE_2
    UTILS_MEX_ASSERT( nlhs == 0, "{}: expected no output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 2, "{}: expected 2 input, nrhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);
    delete ptr;

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_copy(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_3 "TestClassMexWrapper('copy',obj)"
    #define CMD MEX_ERROR_MESSAGE_3
    UTILS_MEX_ASSERT( nlhs == 1, "{}: expected no output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 2, "{}: expected 2 input, nrhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);
    arg_out_0 = Utils::mex_convert_ptr_to_mx<TestClass>(new TestClass(*ptr));

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_a(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_4 "TestClassMexWrapper('a',obj)"
    #define CMD MEX_ERROR_MESSAGE_4
    UTILS_MEX_ASSERT( nlhs == 0, "{}: expected 1 output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 2, "{}: expected 2 input, nrhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);
    mex_set_scalar_value( arg_out_0, ptr->a() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_b(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_5 "TestClassMexWrapper('b',obj)"
    #define CMD MEX_ERROR_MESSAGE_5
    UTILS_MEX_ASSERT( nlhs == 1, "{}: expected 1 output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 2, "{}: expected 2 input, nrhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);
    mex_set_scalar_value( arg_out_0, ptr->b() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_c(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_6 "TestClassMexWrapper('c',obj)"
    #define CMD MEX_ERROR_MESSAGE_6
    UTILS_MEX_ASSERT( nlhs == 1, "{}: expected 1 output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 2, "{}: expected 2 input, nrhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);
    mex_set_scalar_value( arg_out_0, ptr->c() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_set_a(
    int nlhs, mxArray       *[], // unused
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_7 "TestClassMexWrapper('set_a',obj,a)"
    #define CMD MEX_ERROR_MESSAGE_7
    UTILS_MEX_ASSERT( nlhs == 0, "{}: expected no output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 3, "{}: expected 2 input, nlhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);

    double a = mex_get_scalar_value( arg_in_2, CMD ": a" );
    ptr->set_a(a);

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_set_b(
    int nlhs, mxArray       *[], // unused
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_8 "TestClassMexWrapper('set_b',obj,b)"
    #define CMD MEX_ERROR_MESSAGE_8
    UTILS_MEX_ASSERT( nlhs == 0, "{}: expected no output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 3, "{}: expected 2 input, nlhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);

    double b = mex_get_scalar_value( arg_in_2, CMD ": b" );
    ptr->set_b(b);

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_set_c(
    int nlhs, mxArray       *[], // unused
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_9 "TestClassMexWrapper('set_c',obj,c)"
    #define CMD MEX_ERROR_MESSAGE_9
    UTILS_MEX_ASSERT( nlhs == 0, "{}: expected no output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 3, "{}: expected 2 input, nlhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);

    double c = mex_get_scalar_value( arg_in_2, CMD ": c" );
    ptr->set_c(c);

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_info(
    int nlhs, mxArray       *[], // unused
    int nrhs, mxArray const *prhs[]
  ) {
    #define MEX_ERROR_MESSAGE_10 "TestClassMexWrapper('info',obj)"
    #define CMD MEX_ERROR_MESSAGE_10
    UTILS_MEX_ASSERT( nlhs == 0, "{}: expected no output, nlhs = {}\n", CMD, nlhs );
    UTILS_MEX_ASSERT( nrhs == 2, "{}: expected 2 input, nlhs = {}\n", CMD, nrhs );

    TestClass * ptr = Utils::mex_convert_mx_to_ptr<TestClass>(arg_in_1);
    ptr->info( std::cout );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  typedef void (*DO_CMD)( int nlhs, mxArray *plhs[], int nrhs, mxArray const *prhs[] );

  static std::map<std::string,DO_CMD> cmd_to_fun = {
    {"new",do_new},
    {"delete",do_delete},
    {"copy",do_copy},
    {"a",do_a},
    {"b",do_b},
    {"c",do_c},
    {"set_a",do_set_a},
    {"set_b",do_set_b},
    {"set_c",do_set_c},
    {"info",do_info}
  };

#define MEX_ERROR_MESSAGE \
"=====================================================================================\n" \
"FiberMexWrapper: \n" \
"\n" \
"USAGE:\n" \
"  - Constructors:\n" \
"    OBJ = TestClassMexWrapper( 'new' );\n" \
"\n" \
"  On output:\n" \
"    OBJ = pointer to the internal object\n" \
"   " MEX_ERROR_MESSAGE_1  "\n" \
"   " MEX_ERROR_MESSAGE_2  "\n" \
"   " MEX_ERROR_MESSAGE_3  "\n" \
"   " MEX_ERROR_MESSAGE_4  "\n" \
"   " MEX_ERROR_MESSAGE_5  "\n" \
"   " MEX_ERROR_MESSAGE_6  "\n" \
"   " MEX_ERROR_MESSAGE_7  "\n" \
"   " MEX_ERROR_MESSAGE_8  "\n" \
"   " MEX_ERROR_MESSAGE_9  "\n" \
"   " MEX_ERROR_MESSAGE_10 "\n" \
"=====================================================================================\n"

  extern "C"
  void
  mexFunction(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    // the first argument must be a string
    if ( nrhs == 0 ) {
      mexErrMsgTxt(MEX_ERROR_MESSAGE);
      return;
    }

    try {
      UTILS_MEX_ASSERT0( mxIsChar(arg_in_0), "First argument must be a string" );
      std::string cmd = mxArrayToString(arg_in_0);
      DO_CMD pfun = cmd_to_fun.at(cmd);
      pfun( nlhs, plhs, nrhs, prhs );
    } catch ( std::exception const & e ) {
      mexErrMsgTxt( fmt::format( "TestClass Error: {}", e.what() ).c_str() );
    } catch (...) {
      mexErrMsgTxt( "TestClass failed" );
    }
  }
}
