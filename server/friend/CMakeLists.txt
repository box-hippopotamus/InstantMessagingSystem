cmake_minimum_required(VERSION 3.28.3)

set(target "friend_server")
project(${target})

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# 添加项目所需源proto文件
set(proto_path ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(proto_files base.proto friend.proto user.proto message.proto)

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

# =============================== odb ===============================
# 添加项目所需源odb文件
set(odb_path ${CMAKE_CURRENT_SOURCE_DIR}/../odb)
set(odb_files friend_apply.hxx relation.hxx chat_session.hxx session_member.hxx)

set(odb_hxx "")
set(odb_cxx "")
set(odb_srcs "")

# 检查 odb 文件是否存在
foreach(odb_file ${odb_files})
    # xxx.hxx -> xxx-odb.hxx xxx-odb.cxx
    string(REPLACE ".hxx" "-odb.hxx" odb_hxx ${odb_file})
    string(REPLACE ".hxx" "-odb.cxx" odb_cxx ${odb_file})
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}${odb_cxx})
        add_custom_command( # 执行外部命令，编译 odb
            PRE_BUILD
            COMMAND odb
            ARGS -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time ${odb_path}/${odb_file}
            DEPENDS ${odb_path}/${odb_file}
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}
            COMMENT "生成 odb 代码文件: " ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}
        )
    endif()
    list(APPEND odb_srcs ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}) # 编译好的所有obd.cxx文件存起来
endforeach()

# ====================================================================

# 获取所有源码文件
set(src_files "")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source src_files)

# 声明目标和依赖
add_executable(${target} ${src_files} ${proto_srcs} ${odb_srcs})

# 定义宏，传递默认路径
target_compile_definitions(${target} PRIVATE -DDEFAULT_CA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/source/") # CA证书路径

# 设置链接库
target_link_libraries(${target} gflags spdlog fmt brpc ssl crypto protobuf leveldb etcd-cpp-api cpprest curl odb-mysql odb odb-boost cpr elasticlient jsoncpp)

# =============================== 测试 ===============================
# set(test "friend_client")
# set(test_files "")
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/test test_files)
# add_executable(${test} ${test_files} ${proto_srcs})
# target_link_libraries(${test} gflags spdlog fmt brpc ssl crypto protobuf leveldb etcd-cpp-api cpprest curl odb-mysql odb odb-boost cpr elasticlient jsoncpp gtest)
# ====================================================================

# 设置头文件默认搜索路径
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # pb.h pb.cc 等文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common) # 通用组件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include) # 三方库
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../odb) # odb 文件

# 设置安装路径
# INSTALL(TARGETS ${target} RUNTIME DESTINATION bin)