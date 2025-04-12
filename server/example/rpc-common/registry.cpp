#include options"../../common/etcd.hpp"
#include "../../common/logger.hpp"
#include "test.pb.h"

#include <gflags/gflags.h>
#include <brpc/server.h>

#include <thread>

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(instance, "/echo/instance", "当前实例名称");
DEFINE_string(access_hot, "127.0.0.1:7070", "当前示例的外部访问地址");
DEFINE_int32(listen_port, 7070, "rpc服务器监听端口");

class EchoServiceImpl : public example::EchoService
{
public:
    EchoServiceImpl() = default;
    ~EchoServiceImpl() = default;

    virtual void Echo(google::protobuf::RpcController* controller,
        const example::EchoRequest* request,
        example::EchoResponse* response,
        google::protobuf::Closure* done)
    {
        brpc::ClosureGuard rpc_guard(done); 
        // 调用done表示业务处理完毕，此时发送响应报文
        // 为了避免忘记done，使用智能指针管理，生命周期结束时自动done

        std::cout << "收到消息: " << request->message() << std::endl;

        std::string str = request->message() + " 的响应报文!";
        response->set_message(str);

        // done->Run(); 处理完毕
    }
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    // 关闭brpc的默认日志
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
    logging::InitLogging(settings);

    // 构造服务对象
    brpc::Server svr;

    // 向服务对象中新增 EchoServer 服务
    EchoServiceImpl echo_service;
    int ret = svr.AddService(&echo_service, brpc::ServiceOwnership::SERVER_DOESNT_OWN_SERVICE);
    if (ret == -1)
    {
        std::cout << "添加服务失败!" << std::endl;
        return -1;
    }

    // 启动服务器
    brpc::ServerOptions options; // 服务配置项
    options.idle_timeout_sec = -1; // 连接空闲超时时间，如果长期空闲，自动断开连接， -1表示一直维护连接
    options.num_threads = 1; // io线程数量
    ret = svr.Start(FLAGS_listen_port, &);
    if (ret == -1)
    {
        std::cout << "启动服务失败!" << std::endl;
        return -1;
    }

    // 注册服务
    Registry::ptr rclient = std::make_shared<Registry>(FLAGS_etcd_host);
    rclient->registry(FLAGS_base_service + FLAGS_instance, FLAGS_access_hot);

    svr.RunUntilAskedToQuit();

    return 0;
}