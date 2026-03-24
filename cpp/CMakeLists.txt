cmake_minimum_required(VERSION 3.21)
project(opensomeip-examples VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# -------------------------------------------------------------------
# Dependencies
# -------------------------------------------------------------------

FetchContent_Declare(
    opensomeip
    GIT_REPOSITORY https://github.com/vtz/opensomeip.git
    GIT_TAG        962e78d7b58a90001bef673852ea227f236dfb81
)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(opensomeip)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
    GIT_SHALLOW    TRUE
)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
# Avoid custom target name collision with opensomeip's `format` target.
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# -------------------------------------------------------------------
# Output directories
# -------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# -------------------------------------------------------------------
# Shared config loader library
# -------------------------------------------------------------------
add_library(examples-common STATIC common/config.cpp)
target_include_directories(examples-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${opensomeip_SOURCE_DIR}/include
)
target_link_libraries(examples-common PUBLIC yaml-cpp::yaml-cpp)

# Propagate opensomeip include path to all examples
include_directories(${opensomeip_SOURCE_DIR}/include)

# Platform backend includes (POSIX by default)
if(WIN32)
    include_directories(${opensomeip_SOURCE_DIR}/include/platform/win32)
else()
    include_directories(${opensomeip_SOURCE_DIR}/include/platform/posix)
endif()

# -------------------------------------------------------------------
# Examples
# -------------------------------------------------------------------
add_subdirectory(hello_world)
add_subdirectory(method_calls)
add_subdirectory(events)
add_subdirectory(sd_demo)
add_subdirectory(complex_types)
add_subdirectory(large_messages)
add_subdirectory(e2e_protection)
