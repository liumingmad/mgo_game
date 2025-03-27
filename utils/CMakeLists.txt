# 添加静态库
add_library(utils STATIC src/libA.cpp)

# 让外部可以找到头文件
target_include_directories(utils PUBLIC include)