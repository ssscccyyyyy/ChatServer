# 查找 muduo 静态库（.a 文件），在 /usr/local/lib 目录下寻找
find_library(MUDUO_NET muduo_net HINTS /usr/local/lib)   # 查找 libmuduo_net.a
find_library(MUDUO_BASE muduo_base HINTS /usr/local/lib)  # 查找 libmuduo_base.a

# 如果没找到 muduo 库，报错退出
if(NOT MUDUO_NET OR NOT MUDUO_BASE)
    message(FATAL_ERROR "muduo libraries not found in /usr/local/lib")
endif()

add_executable(muduo_server muduo_server.cpp)       # 生成可执行文件: muduo_server
target_link_libraries(muduo_server ${MUDUO_NET} ${MUDUO_BASE} pthread)  # 链接 muduo 网络库 + 线程库
target_include_directories(muduo_server PRIVATE /usr/local/include)     # 添加 muduo 头文件搜索路径
