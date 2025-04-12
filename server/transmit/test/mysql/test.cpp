#include "../../../common/mysql_session_member.hpp"
#include "../../../odb/session_member.hxx"
#include "session_member-odb.hxx"

#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

void append_test(IM::SessionMemberTable& ssmt)
{
    // IM::SessionMember ssm1("ssid1", "uid1");
    // IM::SessionMember ssm2("ssid1", "uid2");
    // IM::SessionMember ssm3("ssid3", "uid3");
    // IM::SessionMember ssm4("ssid3", "uid4");
    // IM::SessionMember ssm5("ssid3", "uid5");
    // IM::SessionMember ssm6("ssid3", "uid6");
    // ssmt.append(ssm1);
    // ssmt.append(ssm2);
    // ssmt.append(ssm3);
    // std::vector<IM::SessionMember> ssms({ssm4, ssm5, ssm6});
    // ssmt.append(ssms);

    IM::SessionMember ssm1("888999", "99457c86e6f50001");
    IM::SessionMember ssm2("888999", "d77d41b174e70004");
    IM::SessionMember ssm3("888999", "1119c72d45aa0005");
    IM::SessionMember ssm4("888999", "52881811c8140002");
    std::vector<IM::SessionMember> ssms({ssm1, ssm2, ssm3, ssm4});
    ssmt.append(ssms);
}

void select_test(IM::SessionMemberTable& ssmt)
{
    auto ret = ssmt.members("ssid3");
    for (auto& id : ret)
        std::cout << id << std::endl;
}

void remove_test(IM::SessionMemberTable& ssmt)
{
    IM::SessionMember ssm1("ssid1", "uid1");
    IM::SessionMember ssm2("ssid1", "uid2");

    ssmt.remove(ssm1);
    ssmt.remove(ssm2);
    ssmt.remove("ssid3");
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    auto db = IM::ODBFactory::create("root", "123456", "127.0.0.1", "IM", "utf8", 3306, 1);
    IM::SessionMemberTable ssmt(db);
    append_test(ssmt);
    // select_test(ssmt);
    // remove_test(ssmt);
    
    return 0;
}