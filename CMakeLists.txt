cmake_minimum_required( VERSION 3.5.1 FATAL_ERROR )
include( FindPkgConfig )

project( system_level )

# server part
set( server_app_name "server" )
set( server_src "src/server.cpp" )

add_executable( ${server_app_name} ${server_src} )

target_compile_options( ${server_app_name} PUBLIC -std=c++11 -Wall -g3 -O0 )

# client part
set( client_app_name "client" )
set( client_src "src/client.cpp" )

add_executable( ${client_app_name} ${client_src} )

target_compile_options( ${client_app_name} PUBLIC -std=c++11 -Wall -g3 -O0 )

