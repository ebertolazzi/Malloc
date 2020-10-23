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
/// file: Utils.hh
///

#pragma once

#ifndef UTILS_dot_HH
#define UTILS_dot_HH

// select computer architecture
#if defined(__APPLE__) && defined(__MACH__)
  // osx architecture
  #define UTILS_OS_OSX 1
  #if defined(__i386__)
    #define UTILS_ARCH32 1
  #elif defined(__x86_64__)
    #define UTILS_ARCH64 1
  #endif
#elif defined(__unix__)
  // linux architecture
  #define UTILS_OS_LINUX 1
  #if defined(__i386__)
    #define UTILS_ARCH32 1
  #elif defined(__x86_64__)
    #define UTILS_ARCH64 1
  #endif
#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
  // windows architecture
  #define UTILS_OS_WINDOWS 1
  #if defined(_M_X64) || defined(_M_AMD64)
    #define UTILS_ARCH64 1
  #else
    #define UTILS_ARCH32 1
  #endif
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  // workaround for windows that may define min and max as macros
  #ifdef min
    #undef min
  #endif
  #ifdef max
    #undef max
  #endif
#else
  #error "unsupported OS!"
#endif

// check if compiler is C++11
#if (defined(_MSC_VER) &&  _MSC_VER >= 1800) || \
    (defined(__cplusplus) && __cplusplus > 199711L)
#else
  #error "Lapack Wrapper must be compiled using C++ >= C++11"
#endif

#define UTILS_PURE_VIRTUAL = 0
#ifndef UTILS_OS_WINDOWS
  #define UTILS_OVERRIDE  override
  #define UTILS_CONSTEXPR constexpr
#else
  #define UTILS_OVERRIDE
  #define UTILS_CONSTEXPR
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wundefined-func-template"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wsigned-enum-bitfield"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wunused-template"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wpadded"
#endif

#include "rang.hpp"
#include "fmt/printf.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <mutex>

#include "Trace.hxx"
#include "Console.hxx"
#include "Malloc.hxx"
#include "Numbers.hxx"
#include "TicToc.hxx"
#include "ThreadPool.hxx"

namespace Utils {
  std::string basename( char const filename[] );
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

///
/// eof: Utils.hh
///
