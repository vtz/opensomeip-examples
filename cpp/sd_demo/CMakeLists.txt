add_executable(sd_demo_server server.cpp)
target_link_libraries(sd_demo_server someip-sd someip-transport examples-common)
