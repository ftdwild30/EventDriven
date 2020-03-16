cmake_minimum_required(VERSION 2.8)
project(EventDriven)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")


set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)
add_subdirectory(test)