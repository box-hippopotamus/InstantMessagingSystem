#include <iostream>
#include <gflags/gflags.h>

// 定义一个布尔类型
// DEFINE_bool(name,  vak,  txt)
//            变量名  默认值 备注

DEFINE_string(ip, "127.0.0.1", "服务器的监听地址");
DEFINE_int32(port, 8080, "服务器的监听端口");
DEFINE_bool(enable_debug, true, "是否启用调试");

// 通过编译参数指定变量： 
// [./test --ip="127.0.0.2" --port=6666 --enable_debug=false]
// --变量名=值

// 通过配置文件指定: 
// [./test --flagfile test.conf]
// 配置文件:
// -变量名=值   (不允许出现多余空格)

// 运行程序前，查看信息
// [./test --help]
// 输出:
// Flags from main.cpp:
// -enable_debug (是否启用调试) type: bool default: true
// -ip (服务器的监听地址) type: string default: "127.0.0.1"
// -port (服务器的监听端口) type: int32 default: 8080

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true); // 初始化参数

    std::cout << FLAGS_ip << std::endl;
    std::cout << FLAGS_port << std::endl;
    std::cout << FLAGS_enable_debug << std::endl;

    return 0;
}