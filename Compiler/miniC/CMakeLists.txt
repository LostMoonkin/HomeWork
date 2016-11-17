cmake_minimum_required(VERSION 3.6)
project(miniC)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

set(SOURCE_FILES
    src/miniC.c)

set(EXECUTABLE_OUTPUT_PATH 
    build)

add_executable(miniC ${SOURCE_FILES})