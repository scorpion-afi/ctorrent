
set( xdu_name "xdu" )
set( xdu_src "xdu/main.cpp" "log.cpp")

pkg_check_modules( boost REQUIRED boost-log )

add_executable( ${xdu_name} ${xdu_src} )

target_compile_options( ${xdu_name} PUBLIC -std=c++11 -Wall -g3 -O0 -Wno-deprecated-declarations
                        ${boost_CFLAGS} )
target_link_libraries( ${xdu_name} ${boost_LDFLAGS} )
target_include_directories( ${xdu_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" )


if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${xdu_name} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
    set_property( TARGET ${xdu_name} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# install rules

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    install( TARGETS ${xdu_name} DESTINATION /system/bin )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    install( TARGETS ${xdu_name} DESTINATION /usr/local/bin )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
