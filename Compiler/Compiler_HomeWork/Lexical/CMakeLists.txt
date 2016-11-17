cmake_minimum_required(VERSION 3.6)
project(lexical)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

set(SOURCE_FILES
    src/lexical.c)

set(EXECUTABLE_OUTPUT_PATH
    build)

add_executable(lexical ${SOURCE_FILES})