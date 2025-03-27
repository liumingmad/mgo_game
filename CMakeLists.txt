cmake_minimum_required(VERSION 3.10)
project(mgo)

# 设置 C++ 版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加子模块
add_subdirectory(common)
add_subdirectory(board)
add_subdirectory(server)
add_subdirectory(main)