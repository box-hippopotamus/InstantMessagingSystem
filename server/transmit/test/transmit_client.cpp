#include "etcd.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <thread>

#include "transmit.pb.h"

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(transmit_service, "/service/transmit_service", "服务监控根目录");

IM::ServiceManager::ptr channel_manager;
IM::UserInfo user_info;

std::string ssid;

IM::ServiceManager::ptr sm;

void string_message(const std::string &uid, const std::string &sid, const std::string &msg) 
{
    auto channel = sm->choose(FLAGS_transmit_service);
    if (!channel) {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }

    IM::MsgTransmitService_Stub stub(channel.get());
    IM::NewMessageReq req;
    IM::GetTransmitTargetRsp rsp;

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid);
    req.set_chat_session_id(sid);
    req.mutable_message()->set_message_type(IM::MessageType::STRING);
    req.mutable_message()->mutable_string_message()->set_content(msg);
    brpc::Controller cntl;
    stub.GetTransmitTarget(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

void image_message(const std::string &uid, const std::string &sid, const std::string &msg)
{
    auto channel = sm->choose(FLAGS_transmit_service);
    if (!channel) {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }

    IM::MsgTransmitService_Stub stub(channel.get());
    IM::NewMessageReq req;
    IM::GetTransmitTargetRsp rsp;

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid);
    req.set_chat_session_id(sid);
    req.mutable_message()->set_message_type(IM::MessageType::IMAGE);
    req.mutable_message()->mutable_image_message()->set_image_content(msg);
    brpc::Controller cntl;
    stub.GetTransmitTarget(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

void speech_message(const std::string &uid, const std::string &sid, const std::string &msg) 
{
    auto channel = sm->choose(FLAGS_transmit_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    
    IM::MsgTransmitService_Stub stub(channel.get());
    IM::NewMessageReq req;
    IM::GetTransmitTargetRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid);
    req.set_chat_session_id(sid);
    req.mutable_message()->set_message_type(IM::MessageType::SPEECH);
    req.mutable_message()->mutable_speech_message()->set_file_contents(msg);
    brpc::Controller cntl;
    stub.GetTransmitTarget(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

void file_message(const std::string &uid, const std::string &sid, 
    const std::string &filename, const std::string &content) 
{
    auto channel = sm->choose(FLAGS_transmit_service);
    if (!channel) {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::MsgTransmitService_Stub stub(channel.get());
    IM::NewMessageReq req;
    IM::GetTransmitTargetRsp rsp;

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid);
    req.set_chat_session_id(sid);
    req.mutable_message()->set_message_type(IM::MessageType::FILE);
    req.mutable_message()->mutable_file_message()->set_file_contents(content);
    req.mutable_message()->mutable_file_message()->set_file_name(filename);
    req.mutable_message()->mutable_file_message()->set_file_size(content.size());
    brpc::Controller cntl;
    stub.GetTransmitTarget(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    sm = std::make_shared<IM::ServiceManager>();
    sm->declared(FLAGS_transmit_service);
    auto put_cb = std::bind(&IM::ServiceManager::onServiceOnline, sm.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&IM::ServiceManager::onServiceOffline, sm.get(), std::placeholders::_1, std::placeholders::_2);

    IM::Discovery::ptr dclient = std::make_shared<IM::Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);
    
    string_message("99457c86e6f50001", "888999", "我是周树人，笔名鲁迅");
    string_message("d77d41b174e70004", "888999", "窗外有爬山虎");
    image_message("1119c72d45aa0005", "888999", "巴金的图片");
    file_message("52881811c8140002", "888999", "一个文件名", "不知名用户上传了一个文件");

    return 0;
}