
set( ctorrent "ctorrent" )
set( ctorrent_src "ctorrent/ctorrent_protocols.cpp" "ctorrent/ctorrent.cpp" )

pkg_check_modules( boost REQUIRED boost-log )

add_library( ${ctorrent} SHARED ${ctorrent_src} )

target_compile_options( ${ctorrent} PUBLIC -std=c++11 -Wall -g3 -O0 -Wno-deprecated-declarations
                        ${boost_CFLAGS} )
target_link_libraries( ${ctorrent} ${boost_LDFLAGS} )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${ctorrent} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
    set_property( TARGET ${ctorrent} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# install rules

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    install( TARGETS ${ctorrent} DESTINATION /system/lib )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    install( TARGETS ${ctorrent} DESTINATION /usr/local/lib )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
