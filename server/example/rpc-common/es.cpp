#include "../../common/icsearch.hpp"

#include <gflags/gflags.h>
#include <memory>

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");


int main(int argc, char* argv[])
{

    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    std::shared_ptr<elasticlient::Client> client(new elasticlient::Client({"http://127.0.0.1:9200/"}));
    
    bool ret = ESIndex(client, "test_user")
            .append("nickname")
            .append("phone", "keyword")
            .create("123456789");

    if (!ret) 
    {
        LOG_ERROR("索引创建失败!");
        return -1;
    }

    // 新增
    ret = ESInsert(client, "test_user")
            .append("nickname", "张三")
            .append("phone", "123123")
            .insert("0001");

    if (!ret) 
    {
        LOG_ERROR("数据插入失败!");
        return -1;
    }

    // 修改
    ret = ESInsert(client, "test_user", "_doc")
            .append("nickname", "张三")
            .append("phone", "456456")
            .insert("0001");

    if (!ret) 
    {
        LOG_ERROR("数据修改失败!");
        return -1;
    }

    // 查询
    Json::Value user = ESSearch(client, "test_user", "_doc")
                        .should_match("phone.keyword", "456456") //.keyword表示phone不需要分词
                        .must_not_term("nickname.keyword", {"张三"})
                        .search();

    if (user.empty() || !user.isArray()) 
    {
        LOG_ERROR("查询结果为空或者有误!");
        // return -1;
    }

    int sz = user.size();
    for (int i = 0; i < sz; i++)
    {
        std::cout << user[i]["_source"]["nickname"].asString() << std::endl;
    }

    // 删除
    ret = ESRemove(client, "test_user", "_doc")
                    .remove("0001");

    if (!ret) 
    { 
        LOG_ERROR("数据删除失败!");
        return -1;
    }

    return 0;
}