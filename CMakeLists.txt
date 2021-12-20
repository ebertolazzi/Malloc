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

include(./cmake/CMakeLists-common.txt)

project(
  Utils
  VERSION ${EB_PROJECT_VERSION}
  HOMEPAGE_URL "https://ebertolazzi.github.io/Utils/"
)

if ( EB_INSTALL_LOCAL )
  set( CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY} )
endif()

message("UTILS current version = v${EB_PROJECT_VERSION}")

include(./cmake/CMakeLists-cflags.txt)
include(./cmake/CMakeLists-utilities.txt)

# Evaluating the suffix to append to target built by this CMake file
ebertolazzi_artifacts_suffix(ARTIFACTS_STATIC_SUFFIX TRUE)
if(BUILD_SHARED)
  ebertolazzi_artifacts_suffix(ARTIFACTS_DYNAMIC_SUFFIX FALSE)
endif()

message( STATUS "Compiler used: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "BASE SUFFIXES = ${ARTIFACTS_STATIC_SUFFIX} ${ARTIFACTS_DYNAMIC_SUFFIX}" )

#   ___
#  / __| ___ _  _ _ _ __ ___ ___
#  \__ \/ _ \ || | '_/ _/ -_|_-<
#  |___/\___/\_,_|_| \__\___/__/
#
set( SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" )

set( SOURCES )
file( GLOB S ${SOURCE_DIR}/*.cc )
foreach(F ${S})
  # build relative path respect to source directory
  file( RELATIVE_PATH RF ${SOURCE_DIR} "${F}" )
  list( APPEND SOURCES ${RF} )
endforeach(F ${S})

#   _  _             _
#  | || |___ __ _ __| |___ _ _ ___
#  | __ / -_) _` / _` / -_) '_(_-<
#  |_||_\___\__,_\__,_\___|_| /__/
#
set( PUBLIC_HEADERS )
file(
  GLOB S
  ${SOURCE_DIR}/*.h*
  ${SOURCE_DIR}/*/*.h*
  ${SOURCE_DIR}/*/*/*.h*
)
foreach (F ${S})
  # build relative path respect to source directory
  file( RELATIVE_PATH RF ${SOURCE_DIR} "${F}" )
  list( APPEND PUBLIC_HEADERS ${RF} )
endforeach(F ${S})

set( PRIVATE_HEADERS )
# no private headers

#list(FILTER HEADERS EXCLUDE REGEX "src\/Eigen\/\.DS")

# extra include directory for compilation
include_directories( ${SOURCE_DIR}/Utils ./lib3rd/include )

#   _____                  _
#  |_   _|_ _ _ _ __ _ ___| |_ ___
#    | |/ _` | '_/ _` / -_)  _(_-<
#    |_|\__,_|_| \__, \___|\__/__/
#                |___/
#
# define target
ebertolazzi_setup_target(
  ${PROJECT_NAME}
  TRUE # build static
  "${SOURCE_DIR}"
  "${SOURCES}"
  "${PUBLIC_HEADERS}"
  "${PRIVATE_HEADERS}"
)

if ( BUILD_SHARED )
  ebertolazzi_setup_target(
    ${PROJECT_NAME}
    FALSE # build dynamic
    "${SOURCE_DIR}"
    "${SOURCES}"
    "${PUBLIC_HEADERS}"
    "${PRIVATE_HEADERS}"
  )
endif()

#   _____       _
#  |_   _|__ __| |_ ___
#    | |/ -_|_-<  _(_-<
#    |_|\___/__/\__/__/
#
if ( BUILD_EXECUTABLE OR ENABLE_CTEST )

  enable_testing()

  set( EXELISTBIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin" )
  set( EXELISTCPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src_tests" )

  if(APPLE OR UNIX)
    find_package( ZLIB REQUIRED )
  else()
    if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
      find_library( ZLIB_LIBRARIES zlibstaticd HINTS ./ThirdParties/zlib/lib )
    else()
      find_library( ZLIB_LIBRARIES zlibstatic HINTS ./ThirdParties/zlib/lib )
    endif()
    set( ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParties/zlib/include ")
  endif()

  file( MAKE_DIRECTORY ${EXELISTBIN_DIR} )

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
    target_include_directories( ${S} PRIVATE ${ZLIB_INCLUDE_DIRS} )
    target_link_libraries( ${S} PRIVATE ebertolazzi::${PROJECT_NAME}_Static ${ZLIB_LIBRARIES} )
    if( UNIX AND NOT APPLE )
      set( THREADS_PREFER_PTHREAD_FLAG ON )
      find_package( Threads REQUIRED )
      target_link_libraries( ${S} PRIVATE ${CMAKE_DL_LIBS} Threads::Threads )
    endif()
    add_test( ${S} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${S} )
    set_target_properties( ${S} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${EXELISTBIN_DIR}" )
  endforeach()
endif()

#   ___         _        _ _
#  |_ _|_ _  __| |_ __ _| | |
#   | || ' \(_-<  _/ _` | | |
#  |___|_||_/__/\__\__,_|_|_|
#
install(
  TARGETS ${TARGETS_TO_BE_INSTALLED}
  # EXPORT (da definire)
  LIBRARY DESTINATION ${INSTALL_PREFIX}/dll
  ARCHIVE DESTINATION ${INSTALL_PREFIX}/lib
  RUNTIME DESTINATION ${INSTALL_PREFIX}/bin
  PUBLIC_HEADER DESTINATION ${PROJECT_PUBLIC_HEADER_DESTINATION}
)

# copy Eigen header only library
install( DIRECTORY "${SOURCE_DIR}/Eigen" DESTINATION ${PROJECT_PUBLIC_HEADER_DESTINATION} )

if ( WIN32 )
  set( DIR3RD ${CMAKE_CURRENT_SOURCE_DIR}/lib3rd )
  install( DIRECTORY "${DIR3RD}/include" DESTINATION "${INSTALL_PREFIX}/" )
  install( DIRECTORY "${DIR3RD}/dll"     DESTINATION "${INSTALL_PREFIX}/" )
  install( DIRECTORY "${DIR3RD}/lib"     DESTINATION "${INSTALL_PREFIX}/" )
endif()

#   ___         _
#  | _ \__ _ __| |____ _ __ _ ___
#  |  _/ _` / _| / / _` / _` / -_)
#  |_| \__,_\__|_\_\__,_\__, \___|
#                       |___/
#

include( ./cmake/CMakeLists-cpack.txt )

ebertolazzi_final_messages()
