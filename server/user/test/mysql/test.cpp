#include "../../../common/mysql_user.hpp"
#include "../../../odb/user.hxx"
#include "user-odb.hxx"

#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

// void insert(IM::UserTable& user_tb)
// {
//     auto user1 = std::make_shared<IM::User>("uid1", "name1", "123456");
//     user_tb.insert(user1);

//     auto user2 = std::make_shared<IM::User>("uid2", "11122223333");
//     user_tb.insert(user2);
// }

void update_by_id(IM::UserTable& user_tb)
{
    auto user = user_tb.select_by_id("uid1");
    user->description("hello world!");
    user_tb.update(user);
}

void update_by_phone(IM::UserTable& user_tb)
{
    auto user = user_tb.select_by_phone("11122223333");
    user->description("hello mysql!");
    user_tb.update(user);
}

void update_by_nickname(IM::UserTable& user_tb)
{
    auto user = user_tb.select_by_nickname("uid2");
    user->password("666999");
    user_tb.update(user);
}

void select_all_users(IM::UserTable& user_tb, const std::vector<std::string>& id_list)
{
    auto users = user_tb.select_by_ids(id_list);
    for (auto& user : users)
    {
        std::cout << user.userId() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    auto db = IM::ODBFactory::create("root", "123456", "127.0.0.1", "IM", "utf8", 3306, 1);
    IM::UserTable user_tb(db);

    // insert(user);
    // update_by_id(user_tb);
    // update_by_phone(user_tb);
    // update_by_nickname(user_tb);
    select_all_users(user_tb, {"uid1", "uid2"});
    return 0;
}