add_executable(hello_world_server server.cpp)
target_link_libraries(hello_world_server someip-transport examples-common)

add_executable(hello_world_client client.cpp)
target_link_libraries(hello_world_client someip-transport examples-common)
