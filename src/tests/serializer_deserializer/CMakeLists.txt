
set( test_binary "serializer_deserializer" )
set( test_src "tests/serializer_deserializer/serializer_deserializer_test.cpp" )

pkg_check_modules( boost REQUIRED boost-log boost-serialization )

add_executable( ${test_binary} ${test_src} )

target_compile_options( ${test_binary} PUBLIC -std=c++11 -Wall -g3 -O0 -Wno-deprecated-declarations
                        ${boost_CFLAGS} )
target_link_libraries( ${test_binary} ${boost_LDFLAGS} -L${CMAKE_CURRENT_BINARY_DIR} -lctorrent ) # libctorrent.so is built as an inner library
target_include_directories( ${test_binary} PUBLIC 
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent"
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent/serializer_deserializer" 
                          ) 

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${test_binary} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
    set_property( TARGET ${test_binary} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# install rules

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    install( TARGETS ${test_binary} DESTINATION /system/bin )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    install( TARGETS ${test_binary} DESTINATION /usr/local/bin )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )


