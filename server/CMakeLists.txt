cmake_minimum_required(VERSION 3.10)
project(server)

# 设置 C++ 版本
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")



aux_source_directory(src SRC_LIST)

# 添加静态库
add_library(server STATIC ${SRC_LIST})

# 链接静态库和动态库
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
target_link_libraries(server PRIVATE OpenSSL::SSL OpenSSL::Crypto ${CURL_LIBRARIES})

target_link_libraries(server PRIVATE common)

# 让外部可以找到头文件
target_include_directories(server PUBLIC include)


option(ENABLE_SERVER_TESTS "Enable tests for server" ON)
if (ENABLE_SERVER_TESTS)
    add_executable(testserver test/test_procotol.cpp)
    target_link_libraries(testserver PRIVATE server common)
endif()