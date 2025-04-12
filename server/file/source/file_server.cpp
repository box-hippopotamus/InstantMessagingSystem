#include "file_server.hpp"

// spdlog
DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

// etcd
DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(instance, "/file_service/instance", "当前实例名称");
DEFINE_string(access_host, "127.0.0.1:9002", "当前实例的外部访问地址");
DEFINE_int32(listen_port, 9002, "rpc服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "rpc超时时间");
DEFINE_int32(rpc_threads, 1, "rpc线程数量");

// file
DEFINE_string(storage_path, "./data", "文件存储路径");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    IM::FileServiceBuilder fsbuilder;
    fsbuilder.init_reg_client(FLAGS_etcd_host, FLAGS_base_service + FLAGS_instance, FLAGS_access_host);
    fsbuilder.init_rpc_server(FLAGS_listen_port, FLAGS_rpc_timeout, FLAGS_rpc_threads, FLAGS_storage_path);
    auto server = fsbuilder.build();
    server->start();

    return 0;
}