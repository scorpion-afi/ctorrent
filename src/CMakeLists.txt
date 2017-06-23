cmake_minimum_required( VERSION 3.5.1 FATAL_ERROR )
include( FindPkgConfig )

project( system_level )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set( ANDROID_BUILD "yes" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

configure_file( ${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h )

# every target has to be notified about where config.h file is
include_directories( ${PROJECT_BINARY_DIR} )

# server part
set( server_app_name "server" )
set( server_src "server.cpp" )

add_executable( ${server_app_name} ${server_src} )

target_compile_options( ${server_app_name} PUBLIC -std=c++11 -Wall -g3 -O0 )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${server_app_name} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${server_app_name} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# client part
set( client_app_name "client" )
set( client_src "client.cpp" )

add_executable( ${client_app_name} ${client_src} )

target_compile_options( ${client_app_name} PUBLIC -std=c++11 -Wall -g3 -O0 -fPIE )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${client_app_name} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${client_app_name} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# install rules

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    install( TARGETS ${server_app_name} ${client_app_name} DESTINATION /system/bin )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    install( TARGETS ${server_app_name} ${client_app_name} DESTINATION /usr/local/bin )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
