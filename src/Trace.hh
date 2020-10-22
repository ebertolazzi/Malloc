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

///
/// file: Trace.hh
///

#pragma once

#ifndef TRACE_dot_HH
#define TRACE_dot_HH

#include "OS.hh"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wsigned-enum-bitfield"
#pragma clang diagnostic ignored "-Wpadded"
#endif

#include "fmt/format.h"
#include "fmt/printf.h"
#include "fmt/chrono.h"
#include "fmt/color.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <mutex>    // std::mutex

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

#ifndef UTILS_ERROR0
  #define UTILS_ERROR0(MSG) \
  throw Utils::Runtime_Error( MSG, __FILE__, __LINE__ )
#endif

#ifndef UTILS_ASSERT0
  #define UTILS_ASSERT0(COND,MSG) if ( !(COND) ) UTILS_ERROR0( MSG )
#endif

#ifndef UTILS_ERROR
  #define UTILS_ERROR(...) \
  throw Utils::Runtime_Error( fmt::format(__VA_ARGS__), __FILE__, __LINE__ )
#endif

#ifndef UTILS_ASSERT
  #define UTILS_ASSERT(COND,...) if ( !(COND) ) UTILS_ERROR( __VA_ARGS__ )
#endif

#ifndef UTILS_ERROR_TRACE0
  #define UTILS_ERROR_TRACE0(MSG) \
  throw Utils::Runtime_TraceError( MSG, __FILE__, __LINE__ )
#endif

#ifndef UTILS_ASSERT_TRACE0
  #define UTILS_ASSERT_TRACE0(COND,MSG) if ( !(COND) ) UTILS_ERROR_TRACE0( MSG )
#endif

#ifndef UTILS_ERROR_TRACE
  #define UTILS_ERROR_TRACE(...) \
  throw Utils::Runtime_TraceError( fmt::format(__VA_ARGS__), __FILE__, __LINE__ )
#endif

#ifndef UTILS_ASSERT_TRACE
  #define UTILS_ASSERT_TRACE(COND,...) if ( !(COND) ) UTILS_ERROR_TRACE( __VA_ARGS__ )
#endif

#ifdef UTILS_NO_DEBUG
  #ifndef UTILS_ASSERT0_DEBUG
    #define UTILS_ASSERT0_DEBUG(COND,MSG)
  #endif
  #ifndef UTILS_ASSERT_DEBUG
    #define UTILS_ASSERT_DEBUG(COND,...)
  #endif
#else
  #ifndef UTILS_ASSERT0_DEBUG
    #define UTILS_ASSERT0_DEBUG(COND,MSG) UTILS_ASSERT0(COND,MSG)
  #endif
  #ifndef UTILS_ASSERT_DEBUG
    #define UTILS_ASSERT_DEBUG(COND,...) UTILS_ASSERT(COND,__VA_ARGS__)
  #endif
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif

namespace Utils {

  typedef std::basic_ostream<char> ostream_type;

  void
  printTrace(
    int                 line,
    char        const   file[],
    std::string const & msg,
    ostream_type      & stream
  );

  class Runtime_TraceError : public std::runtime_error {
  private:
    std::string
    grab_backtrace(
      std::string const & reason,
      char        const   file[],
      int                 line
    ) const;

  public:
    explicit
    Runtime_TraceError( std::string const & reason, char const file[], int line )
    : std::runtime_error( grab_backtrace( reason, file, line ) )
    { }

    virtual const char* what() const noexcept override;
  };

  class Runtime_Error : public std::runtime_error {
  public:
    explicit
    Runtime_Error( std::string const & reason, char const file[], int line )
    : std::runtime_error( fmt::format( "\n{}\nOn File:{}:{}\n", reason, file, line ) )
    { }

    explicit
    Runtime_Error( char const reason[], char const file[], int line )
    : std::runtime_error( fmt::format( "\n{}\nOn File:{}:{}\n", reason, file, line ) )
    { }

    virtual const char* what() const noexcept override;
  };

}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

///
/// eof: lapack_wrapper_utils.hh
///
