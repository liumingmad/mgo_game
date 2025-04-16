aux_source_directory(src SRC_LIST)

# 添加静态库
add_library(board STATIC ${SRC_LIST})

target_link_libraries(board PRIVATE common)

# 让外部可以找到头文件
target_include_directories(board PUBLIC include)

# 仅在构建测试时添加 testA
option(ENABLE_BOARD_TESTS "Enable tests for board" ON)

if (ENABLE_BOARD_TESTS)
    add_executable(testboard test/TestBoard.cpp)
    target_link_libraries(testboard PRIVATE board common)
endif()