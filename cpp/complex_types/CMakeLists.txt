add_executable(complex_types_server server.cpp)
target_link_libraries(complex_types_server someip-rpc examples-common)

add_executable(complex_types_client client.cpp)
target_link_libraries(complex_types_client someip-rpc examples-common)
