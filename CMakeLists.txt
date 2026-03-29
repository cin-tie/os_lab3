cmake_minimum_required(VERSION 3.12)
project(os_lab3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(os_lab3 src/main.cpp src/marker.cpp)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests
    tests/test_marker.cpp
    src/marker.cpp
)

target_include_directories(tests PRIVATE include)

target_link_libraries(tests gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)