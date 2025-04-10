# 添加可执行文件
add_executable(client main.cpp client.cpp)
target_link_libraries(client PRIVATE common)