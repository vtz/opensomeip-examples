cmake_minimum_required(VERSION 3.20.0)

if(NOT DEFINED OPENSOMEIP_ROOT)
    if(DEFINED ENV{OPENSOMEIP_ROOT})
        set(OPENSOMEIP_ROOT $ENV{OPENSOMEIP_ROOT})
    else()
        message(FATAL_ERROR
            "OPENSOMEIP_ROOT is not set. Point it to the opensomeip source tree.\n"
            "  cmake -DOPENSOMEIP_ROOT=/path/to/opensomeip ...")
    endif()
endif()

list(APPEND ZEPHYR_EXTRA_MODULES ${OPENSOMEIP_ROOT})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world_server)

target_sources(app PRIVATE src/main.cpp)
