#include "mysql_message.hpp"
#include <gflags/gflags.h>


DEFINE_bool(run_mode, false, "程序的运行模式，false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

void insert_test(IM::MessageTable &tb) 
{
    IM::Message m1("消息ID1", "888999", "99457c86e6f50001", 0, boost::posix_time::time_from_string("2002-01-20 23:59:59.000"));
    tb.insert(m1);
    IM::Message m2("消息ID2", "888999", "d77d41b174e70004", 0, boost::posix_time::time_from_string("2002-01-21 23:59:59.000"));
    tb.insert(m2);
    IM::Message m3("消息ID3", "888999", "d77d41b174e70004", 0, boost::posix_time::time_from_string("2002-01-22 23:59:59.000"));
    tb.insert(m3);

    IM::Message m4("消息ID4", "888999", "99457c86e6f50001", 0, boost::posix_time::time_from_string("2002-01-20 23:59:59.000"));
    tb.insert(m4);
    IM::Message m5("消息ID5", "888999", "52881811c8140002", 0, boost::posix_time::time_from_string("2002-01-21 23:59:59.000"));
    tb.insert(m5);
}

void remove_test(IM::MessageTable &tb) 
{
    tb.remove("888999");
}

void recent_test(IM::MessageTable &tb) 
{
    auto res = tb.recent("888999", 2);
    auto begin = res.rbegin();
    auto end = res.rend();
    for (; begin != end; ++begin) 
    {
        std::cout << begin->messageId() << std::endl;
        std::cout << begin->sessionId() << std::endl;
        std::cout << begin->userId() << std::endl;
        std::cout << boost::posix_time::to_simple_string(begin->createTime()) << std::endl;
    }
}

void range_test(IM::MessageTable &tb) 
{
    boost::posix_time::ptime stime(boost::posix_time::time_from_string("2002-01-20 23:59:59.000"));
    boost::posix_time::ptime etime(boost::posix_time::time_from_string("2002-01-21 23:59:59.000"));
    auto res = tb.range("888999", stime, etime);
    for (const auto &m : res) 
    {
        std::cout << m.messageId() << std::endl;
        std::cout << m.sessionId() << std::endl;
        std::cout << m.userId() << std::endl;
        std::cout << boost::posix_time::to_simple_string(m.createTime()) << std::endl;
    }
}

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    auto db = IM::ODBFactory::create("root", "123456", "127.0.0.1", "IM", "utf8", 0, 1);
    IM::MessageTable tb(db);
    // insert_test(tb);
    remove_test(tb);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    insert_test(tb);
    recent_test(tb);
    range_test(tb);
    return 0;
}