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

include( ./CMakeLists-common.txt )

project(
  Utils
  VERSION ${EB_PROJECT_VERSION}
  HOMEPAGE_URL "https://ebertolazzi.github.io/Utils/"
)

message("UTILS current version = v${EB_PROJECT_VERSION}")

include(./CMakeLists-cflags.txt)
include(./CMakeLists-utilities.txt)

# Evaluating the suffix to append to target built by this CMake file
ebertolazzi_artifacts_suffix(ARTIFACTS_STATIC_SUFFIX TRUE)
if(BUILD_SHARED)
  ebertolazzi_artifacts_suffix(ARTIFACTS_DYNAMIC_SUFFIX FALSE)
endif()

message( STATUS "Compiler used: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "BASE SUFFIXES = ${ARTIFACTS_STATIC_SUFFIX} ${ARTIFACTS_DYNAMIC_SUFFIX}" )

set( SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" )

set( SOURCES )
file( GLOB S ${SOURCE_DIR}/*.cc )
foreach(F ${S})
  file( RELATIVE_PATH RF ${SOURCE_DIR} "${F}" )
  list( APPEND SOURCES ${SOURCE_DIR}/${RF} )
endforeach(F ${S})

set( HEADERS )
file( GLOB S ${SOURCE_DIR}/*.h* ${SOURCE_DIR}/*/*.h* )
foreach (F ${S})
  file( RELATIVE_PATH RF ${SOURCE_DIR} "${F}" )
  list( APPEND HEADERS ${SOURCE_DIR}/${RF} )
endforeach(F ${S})

include_directories( ${SOURCE_DIR} ${SOURCE_DIR}/Utils )

# define target
ebertolazzi_setup_target( ${PROJECT_NAME} "${SOURCE_DIR}" "${SOURCES}" "${HEADERS}" )

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

install(
  TARGETS ${TARGET_TO_BE_INSTALLED}
  RUNTIME DESTINATION ${INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${INSTALL_PREFIX}/dll
  ARCHIVE DESTINATION ${INSTALL_PREFIX}/lib
  PUBLIC_HEADER DESTINATION ${PROJECT_PUBLIC_HEADER_DESTINATION}
)
install(
  DIRECTORY "${SOURCE_DIR}/" # source directory
  DESTINATION "${INSTALL_PREFIX}/include" # target directory
  FILES_MATCHING # install only matched files
  PATTERN "*.h*" # select header files
)
install(
  DIRECTORY "${SOURCE_DIR}/Eigen"
  DESTINATION "${INSTALL_PREFIX}/include/Eigen"
)

#file(GLOB LIB_ZLIB  ./lib3rd/lib/*.*)
#file(GLOB DLL_ZLIB  ./lib3rd/dll/*.*)
#install(FILES ${LIB_ZLIB}  DESTINATION ${INSTALL_PREFIX}/lib)
#install(FILES ${DLL_ZLIB}  DESTINATION ${INSTALL_PREFIX}/dll)

ebertolazzi_final_messages()
