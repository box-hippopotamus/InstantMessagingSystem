#include "../../common/rabbitmq.hpp"
#include <gflags/gflags.h>

DEFINE_string(user, "root", "RabbitMQ用户名");
DEFINE_string(password, "123456", "RabbitMQ用户密码");
DEFINE_string(host, "127.0.0.1:5672", "RabbitMQ服务器的地址");

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    MQClient client(FLAGS_user, FLAGS_password, FLAGS_host);

    client.declareComponents("test-exchange", "test-queue");

    // 前面的操作的都是异步操作，可能还没有初始化完，直接就发送数据了
    std::this_thread::sleep_for(std::chrono::seconds(3));

    for (int i = 0; i < 10; i++)
    {
        std::string msg = "hello " + std::to_string(i);
        bool ret = client.publish("test-exchange", msg);
        if (!ret)
            std::cout << "发送失败!" << std::endl; 
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(20));

    return 0;
}