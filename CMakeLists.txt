# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)
# 项目名: ChatServer, 语言: C++
project(ChatServer VERSION 1.0.0 LANGUAGES CXX)

# 指定 C++11 标准
set(CMAKE_CXX_STANDARD 11)
# 必须支持 C++11，不兼容就报错
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 生成 compile_commands.json（给 IDE 智能提示用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 默认编译 Debug 模式（可通过 -DCMAKE_BUILD_TYPE=Release 切换）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 全局头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/include)      # 项目公共头文件
include_directories(${CMAKE_SOURCE_DIR}/thirdparty)    # 第三方库头文件

# 可执行文件统一输出到 bin/，库文件统一输出到 lib/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 引入子目录
add_subdirectory(src)
add_subdirectory(test/testjson)
add_subdirectory(test/testmuduo)                    
