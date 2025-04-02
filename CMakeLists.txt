cmake_minimum_required(VERSION 3.10)
project(mgo)

# 设置 C++ 版本
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

# 添加子模块
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(board)
add_subdirectory(server)
add_subdirectory(main)