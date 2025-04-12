#include "etcd.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <thread>
#include "friend.pb.h"

DEFINE_bool(run_mode, false, "程序的运行模式，false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(friend_service, "/service/friend_service", "服务监控根目录");

IM::ServiceManager::ptr sm;

void apply_test(const std::string &uid1, const std::string &uid2) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }

    IM::FriendService_Stub stub(channel.get());
    IM::FriendAddReq req;
    IM::FriendAddRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    req.set_respondent_id(uid2);
    brpc::Controller cntl;
    stub.FriendAdd(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

void get_apply_list(const std::string &uid1) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::GetPendingFriendEventListReq req;
    IM::GetPendingFriendEventListRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    brpc::Controller cntl;
    stub.GetPendingFriendEventList(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    for (int i = 0; i < rsp.event_size(); i++) 
    {
        std::cout << "---------------\n";
        std::cout << rsp.event(i).sender().user_id() << std::endl;
        std::cout << rsp.event(i).sender().nickname() << std::endl;
        std::cout << rsp.event(i).sender().avatar() << std::endl;
    }
}

void process_apply_test(const std::string &uid1, bool agree, const std::string &apply_user_id) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::FriendAddProcessReq req;
    IM::FriendAddProcessRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    req.set_agree(agree);
    req.set_apply_user_id(apply_user_id);
    brpc::Controller cntl;
    stub.FriendAddProcess(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    if (agree) 
    {
        std::cout << rsp.new_session_id() << std::endl;
    }
}

void search_test(const std::string &uid1, const std::string &key) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::FriendSearchReq req;
    IM::FriendSearchRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    req.set_search_key(key);
    brpc::Controller cntl;
    stub.FriendSearch(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    for (int i = 0; i < rsp.user_info_size(); i++) 
    {
        std::cout << "-------------------\n";
        std::cout << rsp.user_info(i).user_id() << std::endl;
        std::cout << rsp.user_info(i).nickname() << std::endl;
        std::cout << rsp.user_info(i).avatar() << std::endl;
    }
}

void friend_list_test(const std::string &uid1) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::GetFriendListReq req;
    IM::GetFriendListRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    brpc::Controller cntl;
    stub.GetFriendList(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    for (int i = 0; i < rsp.friend_list_size(); i++) 
    {
        std::cout << "-------------------\n";
        std::cout << rsp.friend_list(i).user_id() << std::endl;
        std::cout << rsp.friend_list(i).nickname() << std::endl;
        std::cout << rsp.friend_list(i).avatar() << std::endl;
    }
}

void remove_test(const std::string &uid1, const std::string &uid2) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::FriendRemoveReq req;
    IM::FriendRemoveRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    req.set_peer_id(uid2);
    brpc::Controller cntl;
    stub.FriendRemove(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

void create_css_test(const std::string &uid1, const std::vector<std::string> &uidlist)
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::ChatSessionCreateReq req;
    IM::ChatSessionCreateRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    req.set_chat_session_name("快乐一家人");
    for (auto &id : uidlist)
    {
        req.add_member_id_list(id);
    }
    brpc::Controller cntl;
    stub.ChatSessionCreate(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    std::cout << rsp.chat_session_info().chat_session_id() << std::endl;
    std::cout << rsp.chat_session_info().chat_session_name() << std::endl;
}

void cssmember_test(const std::string &uid1, const std::string &cssid) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::GetChatSessionMemberReq req;
    IM::GetChatSessionMemberRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    req.set_chat_session_id(cssid);
    brpc::Controller cntl;
    stub.GetChatSessionMember(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    for (int i = 0; i < rsp.member_info_list_size(); i++) 
    {
        std::cout << "-------------------\n";
        std::cout << rsp.member_info_list(i).user_id() << std::endl;
        std::cout << rsp.member_info_list(i).nickname() << std::endl;
        std::cout << rsp.member_info_list(i).avatar() << std::endl;
    }
}

void csslist_test(const std::string &uid1) 
{
    auto channel = sm->choose(FLAGS_friend_service);
    if (!channel) 
    {
        std::cout << "获取通信信道失败！" << std::endl;
        return;
    }
    IM::FriendService_Stub stub(channel.get());
    IM::GetChatSessionListReq req;
    IM::GetChatSessionListRsp rsp;
    req.set_request_id(IM::UUID::uuid());
    req.set_user_id(uid1);
    brpc::Controller cntl;
    stub.GetChatSessionList(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    for (int i = 0; i < rsp.chat_session_info_list_size(); i++) 
    {
        std::cout << "-------------------\n";
        std::cout << rsp.chat_session_info_list(i).single_chat_friend_id() << std::endl;
        std::cout << rsp.chat_session_info_list(i).chat_session_id() << std::endl;
        std::cout << rsp.chat_session_info_list(i).chat_session_name() << std::endl;
        std::cout << rsp.chat_session_info_list(i).avatar() << std::endl;
        std::cout << "消息内容：\n";
        std::cout << rsp.chat_session_info_list(i).prev_message().message_id() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().chat_session_id() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().timestamp() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().sender().user_id() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().sender().nickname() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().sender().avatar() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().message().file_message().file_name() << std::endl;
        std::cout << rsp.chat_session_info_list(i).prev_message().message().file_message().file_contents() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    sm = std::make_shared<IM::ServiceManager>();
    sm->declared(FLAGS_friend_service);
    auto put_cb = std::bind(&IM::ServiceManager::onServiceOnline, sm.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&IM::ServiceManager::onServiceOffline, sm.get(), std::placeholders::_1, std::placeholders::_2);
    IM::Discovery::ptr dclient = std::make_shared<IM::Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);

    // +----+------------------+-----------+-------------+-----------+-------+------------------+
    // | id | user_id          | nickname  | description | password  | phone | avatar_id        |
    // +----+------------------+-----------+-------------+-----------+-------+------------------+
    // |  1 | 290a2ecb159f0001 | 周树人    | NULL        | 666666    | NULL  | d48a3c66b4030001 |
    // |  2 | 379463a6d7ad0002 | 老舍      | NULL        | aeiouxxx  | NULL  | 8188ba3e975f0002 |
    // |  3 | 8ef98303dd1c0003 | 巴金      | NULL        | bbb999    | NULL  | 28239d72bd6d0003 |
    // |  4 | 4c78e77f13200004 | 李清照    | NULL        | lqz123456 | NULL  | b620a8b67fe00004 |
    // +----+------------------+-----------+-------------+-----------+-------+------------------+

    // apply_test("290a2ecb159f0001", "4c78e77f13200004");
    // apply_test("379463a6d7ad0002", "4c78e77f13200004");
    // apply_test("8ef98303dd1c0003", "4c78e77f13200004");
    // get_apply_list("4c78e77f13200004");
    // process_apply_test("4c78e77f13200004", true, "290a2ecb159f0001");
    // process_apply_test("4c78e77f13200004", false, "379463a6d7ad0002");
    // process_apply_test("4c78e77f13200004", true, "8ef98303dd1c0003");
    std::cout << "*****李清照搜索老舍********\n";
    search_test("4c78e77f13200004", "舍");
    std::cout << "+++++老舍搜索自己++++++++\n";
    search_test("379463a6d7ad0002", "老舍"); // 老舍搜索自己
    std::cout << "++++++李清照搜索好友+++++++\n";
    friend_list_test("4c78e77f13200004"); // 李清照搜索好友
    std::cout << "++++++周树人搜索好友++++++\n";
    friend_list_test("290a2ecb159f0001");
    std::cout << "+++++++老舍搜索好友+++++\n";
    friend_list_test("379463a6d7ad0002");
    // std::cout << "+++++++李清照删除巴金+++++\n";
    // remove_test("4c78e77f13200004", "8ef98303dd1c0003");
    // std::vector<std::string> uidlist = {
    //     "290a2ecb159f0001", 
    //     "379463a6d7ad0002",
    //     "8ef98303dd1c0003",
    //     "4c78e77f13200004"};
    // create_css_test("290a2ecb159f0001", uidlist);
    std::cout << "+++++群聊成员列表++++++++\n";
    cssmember_test("290a2ecb159f0001", "36b5-edaf4987-0000");
    std::cout << "+++++李清照会话列表+++++++\n";
    csslist_test("4c78e77f13200004");
    return 0;
}