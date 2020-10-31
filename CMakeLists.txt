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

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
SET( CMAKE_VERBOSE_MAKEFILE OFF )
SET( CMAKE_INSTALL_MESSAGE NEVER )

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

PROJECT( Utils CXX C )
ENABLE_TESTING()

IF ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  SET( DEBUG_SUFFIX "_debug" )
ELSE()
  SET( DEBUG_SUFFIX )
ENDIF()

IF(APPLE)
  SET( TARGETS Utils_osx_static${DEBUG_SUFFIX} )
ELSEIF( UNIX )
  SET( TARGETS Utils_linux_static${DEBUG_SUFFIX} )
ELSE()
  SET( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )
  SET( TARGETS Utils_win_${BITS}_static${DEBUG_SUFFIX} )
ENDIF()

IF ( BUILD_SHARED )
  IF(APPLE)
    SET( TARGET Utils_osx${DEBUG_SUFFIX} )
  ELSEIF( UNIX )
    SET( TARGET Utils_linux${DEBUG_SUFFIX} )
  ELSE()
    SET( TARGET Utils_win_${BITS}${DEBUG_SUFFIX} )
  ENDIF()
ENDIF()

SET( CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/lib" )

INCLUDE( ./CMakeLists-cflags.txt )

MESSAGE( STATUS "Compiler used: ${CMAKE_CXX_COMPILER_ID}")

SET( CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} )
SET( CMAKE_C_FLAGS_DEBUG   ${CMAKE_CXX_FLAGS_DEBUG} )

SET( SOURCES )
FILE( GLOB S ./src/*.cc )
FOREACH (F ${S})
  FILE( RELATIVE_PATH RF ${CMAKE_CURRENT_SOURCE_DIR} "${F}" )
  LIST( APPEND SOURCES ${RF} )
ENDFOREACH (F ${S})

SET( HEADERS )
FILE( GLOB S ./src/*.h* )
FOREACH (F ${S})
  FILE( RELATIVE_PATH RF ${CMAKE_CURRENT_SOURCE_DIR} "${F}" )
  LIST( APPEND HEADERS ${RF} )
ENDFOREACH (F ${S})
FILE( GLOB S ./src/*/*.h* )
FOREACH (F ${S})
  FILE( RELATIVE_PATH RF ${CMAKE_CURRENT_SOURCE_DIR} "${F}" )
  LIST( APPEND HEADERS ${RF} )
ENDFOREACH (F ${S})
FILE( GLOB S ./src/*/*/*.h* )
FOREACH (F ${S})
  FILE( RELATIVE_PATH RF ${CMAKE_CURRENT_SOURCE_DIR} "${F}" )
  LIST( APPEND HEADERS ${RF} )
ENDFOREACH (F ${S})

ADD_LIBRARY( ${TARGETS} STATIC ${SOURCES} ${HEADERS} )

IF ( BUILD_SHARED )
  ADD_LIBRARY( ${TARGET} SHARED ${SOURCES} ${HEADERS} )
ENDIF()


MESSAGE( STATUS "YEAR = ${YEAR}" )
MESSAGE( STATUS "BITS = ${BITS}" )

INCLUDE_DIRECTORIES( src src/Utils )

IF( BUILD_EXECUTABLE )

  SET( EXELISTCPP test_trace test_Malloc test_Thread )

  ADD_CUSTOM_TARGET( all_tests ALL )

  SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
  FILE(MAKE_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/bin )
  FOREACH( S ${EXELISTCPP} )
  	ADD_EXECUTABLE( ${S} ${CMAKE_CURRENT_SOURCE_DIR}/src_tests/${S}.cc ${HEADERS} )
  	TARGET_LINK_LIBRARIES( ${S} ${TARGETS} ${lapackblas_libraries} )
  	ADD_TEST( ${S} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/${S} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    ADD_DEPENDENCIES( all_tests ${S} )
  ENDFOREACH( S $(EXELIST) )

  FOREACH( S ${EXELISTC} )
  	ADD_EXECUTABLE( ${S} ${CMAKE_CURRENT_SOURCE_DIR}/src_tests/${S}.c ${HEADERS} )
  	TARGET_LINK_LIBRARIES( ${S} ${TARGETS} ${lapackblas_libraries} )
  	ADD_TEST( ${S} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/${S} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    ADD_DEPENDENCIES( all_tests ${S} )
  ENDFOREACH( S $(EXELIST) )

  ADD_CUSTOM_COMMAND(
    TARGET all_tests
    COMMENT "Run tests"
    POST_BUILD COMMAND ctest ARGS --output-on-failure
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

ENDIF()

FILE( GLOB HDR     ./src/*.h* )
FILE( GLOB HDR1    ./src/Utils/*.h* )
FILE( GLOB HDR_FMT ./src/Utils/fmt/*.h* )
FILE( GLOB HDR_ZS  ./src/Utils/zstream/*.h* )

IF ( BUILD_SHARED )
  INSTALL(
    TARGETS ${TARGETHSL} ${TARGETHSLS} ${TARGETS} ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/dll
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  )
ELSE()
  INSTALL(
    TARGETS ${TARGETHSLS} ${TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/dll
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  )
ENDIF()

INSTALL(FILES ${HDR}     DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
INSTALL(FILES ${HDR1}    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Utils )
INSTALL(FILES ${HDR_FMT} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Utils/fmt )
INSTALL(FILES ${HDR_ZS}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Utils/zstream )

MESSAGE( STATUS "Using ${SSE_FLAGS} extensions")
MESSAGE( STATUS "C compiler                  = ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER}" )
MESSAGE( STATUS "C++ compiler                = ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}" )
MESSAGE( STATUS "CMAKE_CXX_FLAGS_RELEASE     = ${CMAKE_CXX_FLAGS_RELEASE}" )
MESSAGE( STATUS "CMAKE_CXX_FLAGS             = ${CMAKE_CXX_FLAGS}" )
MESSAGE( STATUS "CMAKE_C_FLAGS_RELEASE       = ${CMAKE_C_FLAGS_RELEASE}" )
MESSAGE( STATUS "CMAKE_C_FLAGS               = ${CMAKE_C_FLAGS}" )
MESSAGE( STATUS "CMAKE_SYSTEM_NAME           = ${CMAKE_SYSTEM_NAME}" )
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR      = ${CMAKE_SYSTEM_PROCESSOR}" )
MESSAGE( STATUS "EXTRA_LIBS                  = ${EXTRA_LIBS}" )
IF(APPLE)
  MESSAGE( STATUS "CMAKE_OSX_SYSROOT           = ${CMAKE_OSX_SYSROOT}" )
  MESSAGE( STATUS "CMAKE_OSX_ARCHITECTURES     = ${CMAKE_OSX_ARCHITECTURES}" )
  MESSAGE( STATUS "CMAKE_OSX_DEPLOYMENT_TARGET = ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
ENDIF()
