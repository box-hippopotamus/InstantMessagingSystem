cmake_minimum_required(VERSION 3.28.3)

set(target "speech_server")
project(${target})

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# 添加项目所需源proto文件
set(proto_path ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(proto_files speech.proto)

set(proto_cc "")
set(proto_h "")
set(proto_srcs "")

# 检查 proto 文件是否存在
foreach(proto_file ${proto_files})
    string(REPLACE ".proto" ".pb.cc" proto_cc ${proto_file})
    string(REPLACE ".proto" ".pb.h" proto_h ${proto_file})
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}${proto_cc})
        add_custom_command( # 执行外部命令，编译 proto
            PRE_BUILD
            COMMAND protoc
            ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR} -I ${proto_path} ${proto_path}/${proto_file}
            DEPENDS ${proto_path}/${proto_file}
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${proto_cc}
            COMMENT "生成 proto 代码文件: " ${CMAKE_CURRENT_BINARY_DIR}/${proto_cc}
        )
    endif()
    list(APPEND proto_srcs ${CMAKE_CURRENT_BINARY_DIR}/${proto_cc}) # 编译好的所有pb.cc文件存起来
endforeach()

# 获取所有源码文件
set(src_files "")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source src_files)

# 声明目标和依赖
add_executable(${target} ${src_files} ${proto_srcs})

# 设置链接库
target_link_libraries(${target} gflags spdlog fmt brpc ssl crypto protobuf leveldb etcd-cpp-api cpprest curl jsoncpp)

# =============================== 测试 ===============================
# set(test "speech_client")
# set(test_files "")
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/test test_files)
# add_executable(${test} ${test_files} ${proto_srcs})
# target_link_libraries(${test} gflags spdlog fmt brpc ssl crypto protobuf leveldb etcd-cpp-api cpprest curl jsoncpp)
# ====================================================================

# 设置头文件默认搜索路径
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # pb.h pb.cc 等文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common) # 通用组件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include) # 三方库

# 设置安装路径
# INSTALL(TARGETS ${target} RUNTIME DESTINATION bin)