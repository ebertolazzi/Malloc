/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2017                                                      |
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

#include "Utils.hh"

namespace Utils {

  /*\
  :|:  _
  :|: | |__  __ _ ___ ___ _ _  __ _ _ __  ___
  :|: | '_ \/ _` (_-</ -_) ' \/ _` | '  \/ -_)
  :|: |_.__/\__,_/__/\___|_||_\__,_|_|_|_\___|
  :|:
  \*/

  #ifdef UTILS_OS_WINDOWS
    std::string
    basename( char const path[] ) {
      static char drive[100];
      static char dir[1024];
      static char fname[256];
      static char ext[128];
      errno_t e = _splitpath_s(
        path,
        drive, 100,
        dir,   1024,
        fname, 256,
        ext,   128
      );
      UTILS_ASSERT0( e == 0, "lapack_wrapper, basename failed!\n" );
      return fname;
    }
  #else
    std::string
    basename( char const path[] ) {

      if ( path[0] == '\0' ) return std::string("");

      std::string filename(path);

      size_t len   = filename.length();
      size_t index = filename.find_last_of("/\\");

      if ( index == std::string::npos ) return filename;
      if ( index + 1 >= len ) {
        --len;
        index = filename.substr(0, len).find_last_of("/\\");

        if ( len   == 0 ) return filename;
        if ( index == 0 ) return filename.substr(1, len - 1);
        if ( index == std::string::npos ) return filename.substr(0, len);
        return filename.substr(index + 1, len - index - 1);
      }
      return filename.substr(index + 1, len - index);
    }
  #endif

}

///
/// eof: OS.cc
///

