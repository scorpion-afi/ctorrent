cmake_minimum_required( VERSION 3.5.1 FATAL_ERROR )
include( FindPkgConfig )

project( system_level )

set( app_name "system_level" )
set( src "src/main.cpp" )
set( inc_dir "." )

add_executable( ${app_name} ${src} )

target_include_directories( ${app_name} PUBLIC ${inc_dir} )
target_compile_options( ${app_name} PUBLIC -std=c++11 -Wall -g3 -O0 )
target_link_libraries( ${app_name} -lboost_regex )
