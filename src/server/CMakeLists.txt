#定义了一个SRC_LIST变量,包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(db SRC_LIST)
aux_source_directory(model SRC_LIST)
aux_source_directory(redis SRC_LIST)


#指定生成可执行文件
add_executable(ChatServer ${SRC_LIST})

#指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base pthread mysqlclient hiredis)
# hiredis 头文件路径（用于 Redis 发布订阅）
target_include_directories(ChatServer PRIVATE /usr/local/include)