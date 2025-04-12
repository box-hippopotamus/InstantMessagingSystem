#include "message_server.hpp"

// spdlog
DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

// etcd
DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(file_service, "/service/file_service", "文件管理子服务名称");
DEFINE_string(user_service, "/service/user_service", "用户管理子服务名称");
DEFINE_string(instance, "/message_service/instance", "当前实例名称");

DEFINE_string(registry_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(access_host, "127.0.0.1:9006", "当前实例的外部访问地址");
DEFINE_int32(listen_port, 9006, "Rpc服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "Rpc调用超时时间");
DEFINE_int32(rpc_threads, 1, "Rpc的IO线程数量");

// es
DEFINE_string(es_host, "http://127.0.0.1:9200/", "ES搜索引擎服务器URL");

// mysql
DEFINE_string(mysql_host, "127.0.0.1", "Mysql服务器访问地址");
DEFINE_string(mysql_user, "root", "Mysql服务器访问用户名");
DEFINE_string(mysql_pswd, "123456", "Mysql服务器访问密码");
DEFINE_string(mysql_db, "IM", "Mysql默认库名称");
DEFINE_string(mysql_cset, "utf8", "Mysql客户端字符集");
DEFINE_int32(mysql_port, 0, "Mysql服务器访问端口");
DEFINE_int32(mysql_pool_count, 4, "Mysql连接池最大连接数量");

// rabbitMQ
DEFINE_string(mq_user, "root", "RabbitMQ用户");
DEFINE_string(mq_password, "123456", "RabbitMQ密码");
DEFINE_string(mq_host, "127.0.0.1:5672", "RabbitMQ地址");
DEFINE_string(mq_msg_exchange, "msg_exchange", "消息持久化使用的交换机");
DEFINE_string(mq_msg_queue, "msg_queue", "消息持久化使用的队列");
DEFINE_string(mq_msg_bindkey, "msg_bindkey", "消息持久化使用的bindkey");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    IM::MessageServiceBuilder msbuilder;
    
    msbuilder.init_es_client({FLAGS_es_host});
    msbuilder.init_mysql_client(FLAGS_mysql_user, FLAGS_mysql_pswd, FLAGS_mysql_host, 
                                FLAGS_mysql_db, FLAGS_mysql_cset, 
                                FLAGS_mysql_port, FLAGS_mysql_pool_count);

    msbuilder.init_mq_client(FLAGS_mq_user, FLAGS_mq_password, FLAGS_mq_host,
                               FLAGS_mq_msg_exchange, FLAGS_mq_msg_queue, FLAGS_mq_msg_bindkey);
    
    msbuilder.init_discovery_client(FLAGS_registry_host, FLAGS_base_service, 
                                    FLAGS_file_service, FLAGS_user_service);
    msbuilder.init_rpc_server(FLAGS_listen_port, FLAGS_rpc_timeout, FLAGS_rpc_threads);
    msbuilder.init_registry_client(FLAGS_etcd_host, FLAGS_base_service + FLAGS_instance, FLAGS_access_host);

    auto server = msbuilder.build();
    server->start();

    return 0;
}