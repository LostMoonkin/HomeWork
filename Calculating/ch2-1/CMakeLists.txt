cmake_minimum_required(VERSION 3.6)
project(ch2_1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(ch2_1 ${SOURCE_FILES})