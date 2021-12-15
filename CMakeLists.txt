############################################################################
#                                                                          #
#  file: CMakeLists.txt                                                    #
#  Copyright (C) 2016                                                      #
#      Enrico Bertolazzi                                                   #
#      Dipartimento di Ingegneria Industriale                              #
#      Universita` degli Studi di Trento                                   #
#      email: enrico.bertolazzi@ing.unitn.it                               #
#                                                                          #
############################################################################

cmake_minimum_required(VERSION 3.14)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INSTALL_MESSAGE NEVER)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(EB_GIT_VERSION "Get version from GIT command" ON)
option(EB_ENABLE_TESTS "Enable compilation of testing targets" ON)
option(EB_BUILD_SHARED "Enable shared option compilation" ON)
option(EB_TWEAK_VERSION_IS_TIMESTAMP "Sets tweak version to timestamp" OFF)

set(
  EB_NAMESPACE "ebertolazzi"
  CACHE STRING "Sets the namespace used for exports"
)

#
# LIBRARY VERSION
#
# If option enabled, get version from latest git tag This command will not work
# in case of add_subdirectory, fetch content or in general when included from
# another project
set(
  EB_CURRENT_VERSION "1.0"
  CACHE STRING "Default project version, if not taken from git"
)

if(EB_GIT_VERSION)
  # FIXME: Does not work on Windows 10
  execute_process(
    COMMAND git describe --tags --abbrev=0
    OUTPUT_VARIABLE EB_PROJECT_VERSION
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  string(REGEX REPLACE "\n$" "" EB_PROJECT_VERSION "${EB_PROJECT_VERSION}")
else()
  set(EB_BUILD_TIMESTAMP "0")
  if(EB_TWEAK_VERSION_IS_TIMESTAMP)
    string(TIMESTAMP EB_BUILD_TIMESTAMP "%y%m%d%H")
  endif()
  set(EB_PROJECT_VERSION "${EB_CURRENT_VERSION}.${EB_BUILD_TIMESTAMP}")
endif()
message("UTILS current version = v${EB_PROJECT_VERSION}")

#
# DEFAULT SETTINGS
#
# Always include cmath defines globally
add_compile_definitions(_USE_MATH_DEFINES)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE HIGH)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Fixes RPath Problem for Linux dependencies and sub dependencies (easy fix,
# works in 99% of cases)
if(NOT APPLE)
  set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()
# Disable verbose Makefiles on unix
set(CMAKE_VERBOSE_MAKEFILE ON)
# Always export compile commands in json file (this is quite useful for tools
# like Sourcetrail)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  Utils
  VERSION ${EB_PROJECT_VERSION}
  HOMEPAGE_URL "https://ebertolazzi.github.io/Utils/"
)

# NOTE: Predictable target name
set( STATIC_PROJECT_EXPORT  "${PROJECT_NAME}Static" )
set( DYNAMIC_PROJECT_EXPORT "${PROJECT_NAME}" )
set( STATIC_PROJECT         "${EB_NAMESPACE}${STATIC_PROJECT_EXPORT}" )
set( DYNAMIC_PROJECT        "${EB_NAMESPACE}${DYNAMIC_PROJECT_EXPORT}" )

# NOTE: Predictable target name
####set(STATIC_PROJECT "${PROJECT_NAME}Static")
####set(DYNAMIC_PROJECT "${PROJECT_NAME}")

# FIXME: This should refer to functions that acts on targets
include(./CMakeLists-cflags.txt)
include(./CMakeUtilities.txt)

# Evaluating the suffix to append to target built by this CMake file
ebertolazzi_artifacts_suffix(ARTIFACTS_STATIC_SUFFIX TRUE)
if(BUILD_SHARED)
  ebertolazzi_artifacts_suffix(ARTIFACTS_DYNAMIC_SUFFIX FALSE)
endif()

message(STATUS "Compiler used: ${CMAKE_CXX_COMPILER_ID}")
if(NOT UNIX)
  message( STATUS "YEAR = ${YEAR}" )
  message( STATUS "BITS = ${BITS}" )
endif()
message( STATUS "BASE SUFFIXES = ${ARTIFACTS_STATIC_SUFFIX} ${ARTIFACTS_DYNAMIC_SUFFIX}")

# FIXME: This should be applied to target
set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_DEBUG   ${CMAKE_CXX_FLAGS_DEBUG})

# NOTE: Glob shall be avoided. Headers and sources shall be declared
# manually in order to rebuild cmake-cache automatically.
# Ref: https://cmake.org/cmake/help/v3.15/command/file.html?highlight=glob#filesystem
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# "${SOURCE_DIR}/fmt.cc" removed
set(
  SOURCES
  "${SOURCE_DIR}/Console.cc"
  "${SOURCE_DIR}/CPUinfo.cc"
  "${SOURCE_DIR}/Malloc.cc"
  "${SOURCE_DIR}/Numbers.cc"
  "${SOURCE_DIR}/rang.cc"
  "${SOURCE_DIR}/Table.cc"
  "${SOURCE_DIR}/TicToc.cc"
  "${SOURCE_DIR}/Trace.cc"
  "${SOURCE_DIR}/Utils.cc"
  "${SOURCE_DIR}/format.cc"
  "${SOURCE_DIR}/os.cc"
)

# Utils Static Library
add_library(${STATIC_PROJECT} STATIC ${SOURCES})
target_include_directories(
  ${STATIC_PROJECT} PUBLIC
  "$<BUILD_INTERFACE:${SOURCE_DIR}>"
  "$<BUILD_INTERFACE:${SOURCE_DIR}/Utils>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
)
set_target_properties(
  ${STATIC_PROJECT} PROPERTIES
  OUTPUT_NAME "${PROJECT_NAME}${ARTIFACTS_STATIC_SUFFIX}"
  POSITION_INDEPENDENT_CODE ON
)

# Utils Dynamic Library
if(BUILD_SHARED)
  add_library(${DYNAMIC_PROJECT} SHARED ${SOURCES})
  target_include_directories(
    ${DYNAMIC_PROJECT} PUBLIC
    "$<BUILD_INTERFACE:${SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${SOURCE_DIR}/Utils>"
    "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
  )
  set_target_properties(
    ${DYNAMIC_PROJECT} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}${ARTIFACTS_DYNAMIC_SUFFIX}"
    POSITION_INDEPENDENT_CODE ON
  )
  if (WIN32)
    set_target_properties(
      ${DYNAMIC_PROJECT} PROPERTIES
      WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
  endif()
endif()

if(BUILD_EXECUTABLE OR ENABLE_CTEST)
  enable_testing()
  set(EXELISTBIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set(EXELISTCPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src_tests")

  if(APPLE OR UNIX)
    find_package(ZLIB REQUIRED)
  else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      find_library(ZLIB_LIBRARIES zlibstaticd HINTS ./ThirdParties/zlib/lib)
    else()
      find_library(ZLIB_LIBRARIES zlibstatic HINTS ./ThirdParties/zlib/lib)
    endif()
    set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParties/zlib/include")
  endif()

  file(MAKE_DIRECTORY ${EXELISTBIN_DIR})

  set(
    EXELISTCPP
    test_Malloc
    test_Quaternion
    test_rang
    test_table
    test_Thread
    test_trace
    test_zstream
  )

  foreach(S ${EXELISTCPP})
    add_executable( ${S} "${EXELISTCPP_DIR}/${S}.cc")
    target_include_directories(${S} PRIVATE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${S} PRIVATE ${STATIC_PROJECT} ${ZLIB_LIBRARIES})
    if(UNIX AND NOT APPLE)
      set(THREADS_PREFER_PTHREAD_FLAG ON)
      find_package(Threads REQUIRED)
      target_link_libraries(${S} PRIVATE ${CMAKE_DL_LIBS} Threads::Threads)
    endif()
    add_test(${S} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${S})
    set_target_properties(${S} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${EXELISTBIN_DIR}")
  endforeach()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeInstall.txt")

if(SSE_FLAGS)
  message(STATUS "Using ${SSE_FLAGS} extensions")
endif()
message(STATUS "C compiler                  = ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler                = ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE     = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS             = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE       = ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS               = ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_SYSTEM_NAME           = ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR      = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "EXTRA_LIBS                  = ${EXTRA_LIBS}")
if(APPLE)
  message( STATUS "CMAKE_OSX_SYSROOT           = ${CMAKE_OSX_SYSROOT}" )
  message( STATUS "CMAKE_OSX_ARCHITECTURES     = ${CMAKE_OSX_ARCHITECTURES}" )
  message( STATUS "CMAKE_OSX_DEPLOYMENT_TARGET = ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
endif()
message(STATUS "SOURCES = ${SOURCES}")
message(STATUS "HEADERS = ${HEADERS}")