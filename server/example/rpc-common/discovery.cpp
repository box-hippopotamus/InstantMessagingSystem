#include "../../common/etcd.hpp"
#include "../../common/logger.hpp"
#include "../../common/channel.hpp"
#include "test.pb.h"

#include <gflags/gflags.h>

#include <thread>

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(call_service, "/service/echo", "服务监控根目录");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    // 构造rpc信道管理对象
    auto sm = std::make_shared<ServiceManager>();
    sm->declared(FLAGS_call_service);
    auto put_cb = std::bind(&ServiceManager::onServiceOnline, sm.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&ServiceManager::onServiceOffline, sm.get(), std::placeholders::_1, std::placeholders::_2);

    // 构造服务发现对象
    Discovery::ptr dclient = std::make_shared<Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // 获取echo服务的信道
        auto channel = sm->choose(FLAGS_call_service);
        if (!channel)
            continue;
        
        // 发起echo调用
        example::EchoService_Stub stub(channel.get());

        // 调用
        example::EchoRequest req;
        req.set_message("hello world");

        brpc::Controller* cntl = new brpc::Controller();
        example::EchoResponse* rsp = new example::EchoResponse();

        stub.Echo(cntl, &req, rsp, nullptr);
        if (cntl->Failed())
        {
            std::cout << "rpc调用失败: " << cntl->ErrorText() << std::endl;
            delete cntl;
            delete rsp;
            continue;
        }

        std::cout << rsp->message() << std::endl;
        delete cntl;
        delete rsp;
    }

    return 0;
}