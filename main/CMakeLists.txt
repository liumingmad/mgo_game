# 添加可执行文件
add_executable(Mgo main.cpp)

# 链接静态库和动态库
target_link_libraries(Mgo PRIVATE server common)