#include "../../../common/es_user.hpp"
#include "../../../common/logger.hpp"

#include <gflags/gflags.h>

// spdlog
DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

// elsaticsearch
DEFINE_string(es_host, "http://127.0.0.1:9200/", "ElsaticSearch服务地址");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    auto es_client = IM::ESClientFactory::create({FLAGS_es_host});
    auto es_user = std::make_shared<IM::ESUser>(es_client);
    es_user->createIndex();
    // es_user->appendData("用户1", "11155559999", "周12", "hello world", "20232023");
    // es_user->appendData("用户2", "11122223333", "李周周", "hello c", "20242024");
    // es_user->appendData("uid3", "11144447777", "凡456", "hello cpp", "20252025");
    // es_user->appendData("uid4", "11133335555", "霍hh", "hello py", "20262026");
    
    // auto res = es_user->search("周", {"用户1"});
    // for (auto u : res)
    // {
    //     std::cout <<"============================" << std::endl;
    //     std::cout << u.userId() << std::endl;
    //     std::cout << *u.phone() << std::endl;
    //     std::cout << *u.nickname() << std::endl;
    //     std::cout << *u.description() << std::endl;
    //     std::cout << *u.avatarId() << std::endl;
    // }
    return 0;
}
