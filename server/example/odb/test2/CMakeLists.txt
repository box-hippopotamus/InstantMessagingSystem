cmake_minimum_required(VERSION 3.28.3)

project(odb-test)

# 添加项目所需源odb文件
set(odb_path ${CMAKE_CURRENT_SOURCE_DIR}/entity)
set(odb_files student.hxx)

set(odb_hxx "")
set(odb_cxx "")
set(odb_srcs "")

# 检查 odb 文件是否存在
foreach(odb_file ${odb_files})
    # student.hxx -> student-odb.hxx student-odb.cxx
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

# 获取所有源码文件
set(src_files "")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source src_files)

# 声明目标和依赖
add_executable(main ${src_files} ${odb_srcs})

# 设置头文件默认搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/entity)
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # odb.hxx 等文件，生成在执行命令的地方，如果代码有依赖，需要去这个目录找

# 设置要连接的库
target_link_libraries(main -lodb-mysql -lodb -lodb-boost -lgflags)

# 设置安装路径
INSTALL(TARGETS main RUNTIME DESTINATION bin)