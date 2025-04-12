#include "speech_server.hpp"

// asr
DEFINE_string(app_id, "117904044", "app_id");
DEFINE_string(api_key, "h8lNpDJDAOibIrYHJmuiE6c6", "api_key");
DEFINE_string(secret_key, "dPFk7GtT6ZZvupAhtOWfspBQZCsc55jF", "secret_key");

// spdlog
DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

// etcd
DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(instance, "/speech_service/instance", "当前实例名称");
DEFINE_string(access_host, "127.0.0.1:9003", "当前实例的外部访问地址");
DEFINE_int32(listen_port, 9003, "rpc服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "rpc超时时间");
DEFINE_int32(rpc_threads, 1, "rpc线程数量");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    IM::SpeechServiceBuilder ssbuilder;
    ssbuilder.init_asr_client(FLAGS_app_id, FLAGS_api_key, FLAGS_secret_key);
    ssbuilder.init_reg_client(FLAGS_etcd_host, FLAGS_base_service + FLAGS_instance, FLAGS_access_host);
    ssbuilder.init_rpc_server(FLAGS_listen_port, FLAGS_rpc_timeout, FLAGS_rpc_threads);
    auto server = ssbuilder.build();
    server->start();

    return 0;
}