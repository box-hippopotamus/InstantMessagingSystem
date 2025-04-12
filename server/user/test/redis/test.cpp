#include "../../../common/redis_user.hpp"

#include <gflags/gflags.h>

// spdlog
DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

// redis
DEFINE_string(ip, "127.0.0.1", "服务器的监听地址");
DEFINE_int32(port, 6379, "服务器的监听端口");
DEFINE_int32(db, 0, "库的编号");
DEFINE_bool(keep_alive, true, "是否进行长链接保活");

void session_test(const std::shared_ptr<sw::redis::Redis>& client)
{
    IM::Session s(client);
    s.append("会话id1", "用户id1");
    s.append("会话id2", "用户id2");
    s.append("会话id3", "用户id3");
    s.append("会话id4", "用户id4");

    s.remove("会话id2");
    s.remove("会话id3");

    auto res1 = s.uid("会话id1");
    if (res1) std::cout << *res1 << std::endl;

    auto res2 = s.uid("会话id2");
    if (res2) std::cout << *res2 << std::endl;

    auto res3 = s.uid("会话id3");
    if (res3) std::cout << *res3 << std::endl;

    auto res4 = s.uid("会话id4");
    if (res4) std::cout << *res4 << std::endl;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    // IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    auto client = IM::RedisClientFactory::create(FLAGS_ip, FLAGS_port, FLAGS_db, FLAGS_keep_alive);
    session_test(client);

    return 0;
}