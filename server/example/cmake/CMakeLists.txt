# 声明所需的cmake版本
cmake_minimum_required(VERSION 3.0.0)
# 定义项目工程名称
project(test)
# 设置生成目标
add_executable(main main.cpp)

# 设置变量
set(target "main")

# 列表变量
set(src "")
list(APPEND src main.cpp test.cpp)

# 预定义变量
CMAKE_CURRENT_BINARY_DIR # 执行cmake指令所处的dir
CMAKE_CURRENT_SOURCE_DIR # cmakelists.txt 文件所处路径

set(obj "")
# 把 src 里的内容 cpp 替换为 o 存储 obj 中
string(REPLACE ".cpp" ".o" obj ${src})

# 添加头文件路径
include_directories(path)

# 添加链接库
target_link_libraries(main spdlog)

# 错误信息提示
message(STATUS "普通提示")
message(FATAL_ERROR "致命错误，直接退出")

# 源码文件 .cpp .c .cc
set(src_files "")
aux_source_directory() # 将所有源码文件名添加到 src_files 变量

if() # 条件

endif() # 结束条件

foreach(src_file src_files) # 循环
    
endforeach() # 结束循环


