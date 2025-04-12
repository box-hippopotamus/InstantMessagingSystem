#include <iostream>

#include <butil/logging.h>
#include <brpc/server.h>

#include "test.pb.h"

// 子类继承EchoService，实现rpc业务调用

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
    ret = svr.Start(6666, &options);
    if (ret == -1)
    {
        std::cout << "启动服务失败!" << std::endl;
        return -1;
    }

    svr.RunUntilAskedToQuit();

    return 0;
}