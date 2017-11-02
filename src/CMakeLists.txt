cmake_minimum_required( VERSION 3.5.1 FATAL_ERROR )
include( FindPkgConfig )

project( ctorrent ) 

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set( ANDROID_BUILD "yes" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# generate ${PROJECT_BINARY_DIR}/config.h file
configure_file( ${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h )

# every target has to be notified about where config.h file is
include_directories( ${PROJECT_BINARY_DIR} )

include( ctorrent/CMakeLists.txt )
include( xdu/CMakeLists.txt )
include( task_solver/CMakeLists.txt )
