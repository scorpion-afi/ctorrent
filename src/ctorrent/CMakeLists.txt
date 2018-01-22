
set( ctorrent "ctorrent" )
set( ctorrent_src "ctorrent/id_generator.cpp" "ctorrent/ctorrent_protocols.cpp"
                  "ctorrent/epoll_event_loop.cpp"
                  "ctorrent/remote_connection.cpp"
                  "ctorrent/ctorrent_client/ctorrent_client.cpp" 
                  "ctorrent/ctorrent_client/remote_server.cpp"
                  "ctorrent/ctorrent_server/ctorrent_server.cpp" 
                  "ctorrent/ctorrent_server/remote_client.cpp" 
                  "ctorrent/ctorrent_server/executor_thread.cpp" 
                  "ctorrent/ctorrent_server/send_thread.cpp" 
                  "ctorrent/serializer_deserializer/serializer.cpp"
                  "ctorrent/serializer_deserializer/deserializer.cpp"
                  "ctorrent/serializer_deserializer/serializer_deserializer.cpp")

pkg_check_modules( boost REQUIRED boost-log boost-serialization )

add_library( ${ctorrent} SHARED ${ctorrent_src} )

target_compile_options( ${ctorrent} PUBLIC -std=c++11 -Wall -g3 -O0 -Wno-deprecated-declarations
                        ${boost_CFLAGS} )
target_link_libraries( ${ctorrent} ${boost_LDFLAGS} -ldl )
target_include_directories( ${ctorrent} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent" 
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent/serializer_deserializer"
                          ) 

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${ctorrent} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
    set_property( TARGET ${ctorrent} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# install rules

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    install( TARGETS ${ctorrent} DESTINATION /system/lib )
    #install( FILES ctorrent/ctorrent_protocols.h DESTINATION /usr/local/include )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    install( TARGETS ${ctorrent} DESTINATION /usr/local/lib )
    
    # to compile a task (task.cpp) into a dynamically loadable library
    install( FILES ctorrent/ctorrent_protocols.h DESTINATION /usr/local/include )
    install( FILES ctorrent/serializer_deserializer/serializer_deserializer.h DESTINATION /usr/local/include )
    install( FILES ctorrent/object.h DESTINATION /usr/local/include )
    install( FILES ctorrent/id_generator.h DESTINATION /usr/local/include )
    
    install( FILES ctorrent/ctorrent_client/servers_list.txt DESTINATION /usr/local/share/ctorrent )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

