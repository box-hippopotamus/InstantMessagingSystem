#include "etcd.hpp"
#include "channel.hpp"
#include <gflags/gflags.h>
#include <thread>
#include "asr.hpp"
#include "speech.pb.h"

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(speech_service, "/service/speech_service", "服务监控根目录");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    // 构造rpc信道管理对象
    auto sm = std::make_shared<IM::ServiceManager>();
    sm->declared(FLAGS_speech_service);
    auto put_cb = std::bind(&IM::ServiceManager::onServiceOnline, sm.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&IM::ServiceManager::onServiceOffline, sm.get(), std::placeholders::_1, std::placeholders::_2);

    // 构造服务发现对象
    IM::Discovery::ptr dclient = std::make_shared<IM::Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);

    // 获取服务的信道
    auto channel = sm->choose(FLAGS_speech_service);
    if (!channel)
        return -1;
    
    // 发起调用
    IM::SpeechService_Stub stub(channel.get());

    std::string file_content;
    aip::get_file_content("16k.pcm", &file_content);
    std::cout << file_content.size() << std::endl;

    IM::SpeechRecognitionReq req;
    req.set_speech_content(file_content);
    req.set_request_id("123456");

    brpc::Controller* cntl = new brpc::Controller();
    IM::SpeechRecognitionRsp* rsp = new IM::SpeechRecognitionRsp();

    stub.SpeechRecognition(cntl, &req, rsp, nullptr);
    if (cntl->Failed())
    {
        std::cout << "rpc调用失败: " << cntl->ErrorText() << std::endl;
        delete cntl;
        delete rsp;
        return 0;
    }

    if (rsp->success())
    {
        std::cout << rsp->request_id() << std::endl;
        std::cout << rsp->recognition_result() << std::endl;
    }
    else 
    {
        std::cout << rsp->errmsg() << std::endl;
    }

    delete cntl;
    delete rsp;

    return 0;
}