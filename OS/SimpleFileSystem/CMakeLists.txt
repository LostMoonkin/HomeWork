cmake_minimum_required(VERSION 3.6)
project(SimpleFileSystem)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m32")

set(SOURCE_FILES
        src/Main.cpp src/MyExt.cpp src/header/MyExt.h src/header/SuperBlock.h src/header/INode.h src/header/FCB.h src/header/User.h src/header/MD5.h src/MD5.cpp)

set(EXECUTABLE_OUTPUT_PATH
        build)

add_executable(SimpleFileSystem ${SOURCE_FILES})