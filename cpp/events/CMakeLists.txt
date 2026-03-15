add_executable(events_publisher publisher.cpp)
target_link_libraries(events_publisher someip-events examples-common)

add_executable(events_subscriber subscriber.cpp)
target_link_libraries(events_subscriber someip-events examples-common)
