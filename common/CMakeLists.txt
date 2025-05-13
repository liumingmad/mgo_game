find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

aux_source_directory(src SRC_LIST)

# 添加静态库
add_library(common STATIC ${SRC_LIST})
target_link_libraries(common PRIVATE crypto++ ssl spdlog fmt)


# 让外部可以找到头文件
target_include_directories(common PUBLIC include)

