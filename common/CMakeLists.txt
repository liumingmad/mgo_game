
# 添加静态库
add_library(common STATIC src/cryptoUtils.cpp src/Log.cpp)
target_link_libraries(common PRIVATE crypto++ ssl)


# 让外部可以找到头文件
target_include_directories(common PUBLIC include)

