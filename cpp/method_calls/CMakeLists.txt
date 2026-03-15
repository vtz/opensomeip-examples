add_executable(method_calls_server server.cpp)
target_link_libraries(method_calls_server someip-rpc examples-common)

add_executable(method_calls_client client.cpp)
target_link_libraries(method_calls_client someip-rpc examples-common)
