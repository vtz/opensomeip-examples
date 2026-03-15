add_executable(large_messages_server server.cpp)
target_link_libraries(large_messages_server someip-rpc someip-tp examples-common)

add_executable(large_messages_client client.cpp)
target_link_libraries(large_messages_client someip-rpc someip-tp examples-common)
