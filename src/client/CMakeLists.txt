# 客户端可执行文件
add_executable(ChatClient main.cpp)

# 链接 muduo 基础库（用于日志）和 pthread
target_link_libraries(ChatClient muduo_base pthread)
