cmake_minimum_required(VERSION 3.12)
project(os_lab3)

set(CMAKE_CXX_STANDART 11)
set(CMAKE_CXX_STANDART_REQUIRED ON)

include_directories($(CMAKE_SOURCE_DIR)/include)

add_executable(os_lab3 src/main.cpp src/marker.cpp)