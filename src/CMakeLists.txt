cmake_minimum_required( VERSION 3.5.1 FATAL_ERROR )
include( FindPkgConfig )

project( ctorrent ) 

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set( ANDROID_BUILD "yes" )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set( X86_64_BUILD "yes" )
endif()

option( DEBUG "To control the debug information generation" OFF )
option( TESTS "Whether build tests or not" ON )

# generate ${PROJECT_BINARY_DIR}/config.h file
configure_file( ${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h )

# every target has to be notified about where config.h file is
include_directories( ${PROJECT_BINARY_DIR} )

include( ctorrent/CMakeLists.txt )
include( xdu/CMakeLists.txt )
include( task_solver/CMakeLists.txt )

# tests need the libctorrent library, so there's no way to build/launch them before, at least now...
if( ${TESTS} )
  include( tests/CMakeLists.txt )
endif()
