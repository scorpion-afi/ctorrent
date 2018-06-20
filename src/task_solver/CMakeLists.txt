
set( task_solver_name "task_solver" )
set( task_solver_src "task_solver/main.cpp" "task_solver/hash_calculator.cpp" "task_solver/string_invertor.cpp" "log.cpp" )

pkg_check_modules( boost REQUIRED boost-log )

add_executable( ${task_solver_name} ${task_solver_src} )

target_compile_options( ${task_solver_name} PUBLIC -std=c++17 -Wall -g3 -O0 -Wno-deprecated-declarations
                        ${boost_CFLAGS} )
target_link_libraries( ${task_solver_name} ${boost_LDFLAGS} -L${CMAKE_CURRENT_BINARY_DIR} -lctorrent ) # libctorrent.so is built as an inner library
target_include_directories( ${task_solver_name} PUBLIC
                            "${CMAKE_CURRENT_SOURCE_DIR}"
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent"
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent/ctorrent_client"
                            "${CMAKE_CURRENT_SOURCE_DIR}/ctorrent/serializer_deserializer"
                          )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    set_property( TARGET ${task_solver_name} APPEND_STRING PROPERTY COMPILE_OPTIONS "-fPIE" )
    set_property( TARGET ${task_solver_name} APPEND_STRING PROPERTY LINK_FLAGS "-pie" )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )

# install rules

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    install( TARGETS ${task_solver_name} DESTINATION /system/bin )
    #install( FILES task_solver/task.cpp DESTINATION /usr/local/share/task_solver )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    install( TARGETS ${task_solver_name} DESTINATION /usr/local/bin )
    install( FILES task_solver/task_hash.cpp DESTINATION /usr/local/share/task_solver )
    install( FILES task_solver/task_reverse.cpp DESTINATION /usr/local/share/task_solver )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
