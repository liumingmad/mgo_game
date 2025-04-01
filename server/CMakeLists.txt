aux_source_directory(src SRC_LIST)

# 添加静态库
add_library(server STATIC ${SRC_LIST})

target_link_libraries(server PRIVATE common)

# 让外部可以找到头文件
target_include_directories(server PUBLIC include)