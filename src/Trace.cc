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

#ifndef UTILS_OS_WINDOWS
#include <execinfo.h> // for backtrace
#include <dlfcn.h>    // for dladdr
#include <cxxabi.h>   // for __cxa_demangle
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wpoison-system-directories"
#pragma clang diagnostic ignored "-Wc++98-compat"
#endif

namespace Utils {

  const char*
  Runtime_Error::what() const noexcept {
    return std::runtime_error::what();
  }

  const char*
  Runtime_TraceError::what() const noexcept {
    return std::runtime_error::what();
  }

  #ifdef UTILS_OS_WINDOWS

  void
  printTrace(
    int                 line,
    char        const   file[],
    std::string const & msg,
    ostream_type & stream
  ) {
    fmt::print( stream,
      "---------------------------------------------------------\n"
      "file: {}:{}\n{}\n"
      "---------------------------------------------------------\n",
      file, line, msg
    );
  }

  /*
    #include "StackWalker.h"
    static
    inline
    void
    printStackTrace( FILE *out = stderr ) {
      fprintf( out, "stack trace:\n" );
      StackWalker sw;
      sw.ShowCallstack();
    }
  */
  std::string
  Runtime_TraceError::grab_backtrace(
    std::string const & reason,
    char const          file[],
    int                 line
  ) const {
    return fmt::format( "\n{}\nOn File:{}:{}\n", reason, file, line );
  }

  #else

  static
  inline
  std::string
  demang( char const mangled_name[] ) {
    int status = 0 ;
    std::string retval = mangled_name;
    char * name = abi::__cxa_demangle( mangled_name, nullptr, nullptr, &status );
    if ( status == 0 ) retval = name;
    if ( name != nullptr ) std::free(name) ;
    return retval;
  }
  //! print a trace stack used in debug
  void
  printTrace(
    int                 line,
    char        const   file[],
    std::string const & reason,
    ostream_type      & stream
  ) {

    fmt::print(
      stream, "\n{}\nOn File:{}:{}\nprocess ID:{}, parent process ID:{}\nstack trace:\n",
      reason, basename(file), line, getpid(), getppid()
    );

    //  record stack trace upto 128 frames
    void *callstack[128] = {};

    // collect stack frames
    int frames = backtrace( callstack, 128);

    // get the human-readable symbols (mangled)
    char** strs = backtrace_symbols( callstack, frames );

    for ( int i = 1; i < frames; ++i) {
      #ifdef UTILS_OS_LINUX
        Dl_info dlinfo;
        if( !dladdr(callstack[i], &dlinfo) ) continue;
        fmt::print( stream, "{:2} {}\n", i, demang( dlinfo.dli_sname ) );
      #else
        char functionSymbol[1024] = {};
        char moduleName[1024]     = {};
        int  offset               = 0;
        char addr[48]             = {};
        // split the string, take out chunks out of stack trace
        // we are primarily interested in module, function and address
        sscanf(
          strs[i], "%*s %s %s %s %*s %d",
          moduleName, addr, functionSymbol, &offset
        );
        //  if this is a C++ library, symbol will be demangled
        //  on success function returns 0
        //
        fmt::print( stream, "{:2} {:30}  [{}] {} + {}\n",
          i, moduleName, addr, demang( functionSymbol ), offset
        );
      #endif
    }
    free(strs);
  }

  std::string
  Runtime_TraceError::grab_backtrace(
    std::string const & reason,
    char const          file[],
    int                 line
  ) const {
    std::ostringstream ost;
    printTrace( line, file, reason, ost );
    return ost.str();
  }
  #endif

}

///
/// eof: Trace.cc
///

