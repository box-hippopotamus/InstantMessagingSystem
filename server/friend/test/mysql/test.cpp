#include "mysql_chat_session.hpp"
#include "mysql_friend_apply.hpp"
#include "mysql_relation.hpp"

#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序的运行模式，false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

void r_insert_test(IM::RelationTable& tb)
{
    tb.insert("99457c86e6f50001", "d77d41b174e70004");
    tb.insert("52881811c8140002", "1119c72d45aa0005");
}

void r_select_test(IM::RelationTable& tb) 
{
    auto res = tb.friends("99457c86e6f50001");
    for (auto &uid:res) 
    {
        std::cout << uid << std::endl;
    }
}

void r_remove_test(IM::RelationTable& tb) 
{
    tb.remove("99457c86e6f50001", "d77d41b174e70004");
}
 
void r_exists_test(IM::RelationTable& tb) 
{
    std::cout << tb.exists("99457c86e6f50001", "d77d41b174e70004") << std::endl;
    std::cout << tb.exists("52881811c8140002", "1119c72d45aa0005") << std::endl;
}

void a_insert_test(IM::FriendApplyTable& tb) 
{
    IM::FriendApply fa1("uuid1", "99457c86e6f50001", "d77d41b174e70004");
    tb.insert(fa1);
    
    IM::FriendApply fa2("uuid2", "99457c86e6f50001", "52881811c8140002");
    tb.insert(fa2);

    IM::FriendApply fa3("uuid3", "99457c86e6f50001", "1119c72d45aa0005");
    tb.insert(fa3);
}

void a_remove_test(IM::FriendApplyTable& tb) 
{
    tb.remove("99457c86e6f50001", "1119c72d45aa0005");
}

void a_select_test(IM::FriendApplyTable& tb) 
{
    // IM::FriendApply fa3("uuid3", "用户ID2", "用户ID3");
    // tb.insert(fa3);

    auto res = tb.apply_users("52881811c8140002");
    for (auto &uid:res) {
        std::cout << uid << std::endl;
    }
}

void a_exists_test(IM::FriendApplyTable& tb) 
{
    std::cout << tb.exists("99457c86e6f50001", "d77d41b174e70004") << std::endl;
    std::cout << tb.exists("99457c86e6f50001", "52881811c8140002") << std::endl;
    std::cout << tb.exists("99457c86e6f50001", "1119c72d45aa0005") << std::endl;
}

void c_insert_test(IM::ChatSessionTable& tb)
{
    // IM::ChatSession cs1("222333", "会话名称1", IM::ChatSessionType::SINGLE);
    // tb.insert(cs1);

    // IM::ChatSession cs2("888999", "会话名称2", IM::ChatSessionType::GROUP);
    // tb.insert(cs2);
    
    IM::ChatSession cs3("555666", "会话名称3", IM::ChatSessionType::SINGLE);
    tb.insert(cs3);
    
    IM::ChatSession cs4("666777", "会话名称4", IM::ChatSessionType::SINGLE);
    tb.insert(cs4);

    IM::ChatSession cs5("777888", "会话名称5", IM::ChatSessionType::GROUP);
    tb.insert(cs5);
}

void c_select_test(IM::ChatSessionTable& tb)
{
    auto res = tb.select("888999");
    std::cout << res->sessionId() << std::endl;
    std::cout << res->sessionName() << std::endl;
    std::cout << (int)res->sessionType() << std::endl;
}       

void c_single_test(IM::ChatSessionTable& tb) 
{
    auto res = tb.single_chat_session("99457c86e6f50001");
    for (auto &info : res) 
    {
        std::cout << info._session_id << std::endl;
        std::cout << info._friend_id << std::endl;
    }
}

void c_group_test(IM::ChatSessionTable& tb) 
{
    auto res = tb.group_chat_session("d77d41b174e70004");
    for (auto &info : res) 
    {
        std::cout << info._session_id << std::endl;
        std::cout << info._session_name << std::endl;
    }
}

void c_remove_test(IM::ChatSessionTable& tb)
{
    tb.remove("777888");
}

void c_remove_test2(IM::ChatSessionTable& tb) 
{
    tb.remove("99457c86e6f50001", "d77d41b174e70004");
}

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    auto db = IM::ODBFactory::create("root", "123456", "127.0.0.1", "IM", "utf8", 0, 1);
    IM::RelationTable rtb(db);
    IM::FriendApplyTable ftb(db);
    IM::ChatSessionTable ctb(db);

    // r_insert_test(rtb);
    // r_select_test(rtb);
    // r_remove_test(rtb);
    // r_exists_test(rtb);
    // a_insert_test(ftb);
    // std::cout << "--------------\n";
    // a_remove_test(ftb);
    // std::cout << "--------------\n";
    // a_select_test(ftb);
    // std::cout << "--------------\n";
    // a_exists_test(ftb);
    c_insert_test(ctb);
    c_select_test(ctb);
    std::cout << "single --------------\n";
    c_single_test(ctb);
    std::cout << "group --------------\n";
    c_group_test(ctb);
    c_remove_test(ctb);
    c_remove_test2(ctb);

    return 0;
}