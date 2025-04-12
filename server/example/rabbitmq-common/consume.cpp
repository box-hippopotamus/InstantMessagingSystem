#include "../../common/rabbitmq.hpp"
#include <gflags/gflags.h>
#include <thread>

DEFINE_string(user, "root", "RabbitMQ用户名");
DEFINE_string(password, "123456", "RabbitMQ用户密码");
DEFINE_string(host, "127.0.0.1:5672", "RabbitMQ服务器的地址");

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

void callback(const char* body, size_t sz)
{
    std::string msg;
    msg.assign(body, sz);
    std::cout << msg << std::endl;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    MQClient client(FLAGS_user, FLAGS_password, FLAGS_host);

    client.declareComponents("test-exchange", "test-queue");
    client.consume("test-queue", callback);

    std::this_thread::sleep_for(std::chrono::seconds(20));

    return 0;
}