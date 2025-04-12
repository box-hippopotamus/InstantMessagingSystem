#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <thread>

#include "utils.hpp"
#include "etcd.hpp"
#include "channel.hpp"
#include "base.pb.h"
#include "user.pb.h"

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(user_service, "/service/user_service", "服务监控根目录");

IM::ServiceManager::ptr channel_manager;
IM::UserInfo user_info;

std::string ssid;

// // 注册测试
// TEST(user_test1, regist)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserRegisterReq req;
//     IM::UserRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname("周树人");
//     req.set_password("666666");

//     stub.UserRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// TEST(user_test2, regist)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserRegisterReq req;
//     IM::UserRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname("老舍");
//     req.set_password("aeiouxxx");

//     stub.UserRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// TEST(user_test3, regist)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserRegisterReq req;
//     IM::UserRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname("巴金");
//     req.set_password("bbb999");

//     stub.UserRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// TEST(user_test4, regist)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserRegisterReq req;
//     IM::UserRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname("李清照");
//     req.set_password("lqz123456");

//     stub.UserRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// TEST(user_test, regist2)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserRegisterReq req;
//     IM::UserRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname("老舍");
//     req.set_password("abcdef");

//     stub.UserRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// TEST(user_test, regist3)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserRegisterReq req;
//     IM::UserRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname("巴金");
//     req.set_password("gfd_asd123");

//     stub.UserRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// // 登录测试
// TEST(user_test, login)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::UserLoginReq req;
//     IM::UserLoginRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_nickname(user_info.nickname());
//     req.set_password("123456");

//     stub.UserLogin(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
//     ssid = rsp.login_session_id();
// }

// 上传头像测试
TEST(user_test, avatar1)
{
    auto channel = channel_manager->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    IM::UserService_Stub stub(channel.get());
    IM::SetUserAvatarReq req;
    IM::SetUserAvatarRsp rsp;
    brpc::Controller* cntl = new brpc::Controller();

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id("290a2ecb159f0001");
    req.set_session_id("123");
    req.set_avatar("周树人的头像");

    stub.SetUserAvatar(cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(user_test, avatar2)
{
    auto channel = channel_manager->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    IM::UserService_Stub stub(channel.get());
    IM::SetUserAvatarReq req;
    IM::SetUserAvatarRsp rsp;
    brpc::Controller* cntl = new brpc::Controller();

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id("379463a6d7ad0002");
    req.set_session_id("456");
    req.set_avatar("老舍的头像");

    stub.SetUserAvatar(cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(user_test, avatar3)
{
    auto channel = channel_manager->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    IM::UserService_Stub stub(channel.get());
    IM::SetUserAvatarReq req;
    IM::SetUserAvatarRsp rsp;
    brpc::Controller* cntl = new brpc::Controller();

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id("8ef98303dd1c0003");
    req.set_session_id("321456");
    req.set_avatar("巴金的头像");

    stub.SetUserAvatar(cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(user_test, avatar4)
{
    auto channel = channel_manager->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    IM::UserService_Stub stub(channel.get());
    IM::SetUserAvatarReq req;
    IM::SetUserAvatarRsp rsp;
    brpc::Controller* cntl = new brpc::Controller();

    req.set_request_id(IM::UUID::uuid());
    req.set_user_id("4c78e77f13200004");
    req.set_session_id("888");
    req.set_avatar("李清照的头像");

    stub.SetUserAvatar(cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp.success());
}

// // 上传昵称测试
// TEST(user_test, nickname)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::SetUserNicknameReq req;
//     IM::SetUserNicknameRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_user_id(user_info.user_id());
//     req.set_session_id(ssid);
//     req.set_nickname("周树人");

//     stub.SetUserNickname(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// // 上传描述信息测试
// TEST(user_test, description)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::SetUserDescriptionReq req;
//     IM::SetUserDescriptionRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_user_id(user_info.user_id());
//     req.set_session_id(ssid);
//     req.set_description(user_info.description());

//     stub.SetUserDescription(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// // 获取用户信息测试
// TEST(user_test, get_info)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::GetUserInfoReq req;
//     IM::GetUserInfoRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_user_id(user_info.user_id());
//     req.set_session_id(ssid);

//     stub.GetUserInfo(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());

//     ASSERT_EQ("周树人", rsp.user_info().nickname());
//     ASSERT_EQ(user_info.user_id(), rsp.user_info().user_id());
//     ASSERT_EQ(user_info.description(), rsp.user_info().description());
//     ASSERT_EQ("", rsp.user_info().phone());
//     ASSERT_EQ(user_info.avatar(), rsp.user_info().avatar());
// }

// // 获取多个用户信息测试
// TEST(user_test, get_multi_info)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::UserService_Stub stub(channel.get());
//     IM::GetMultiUserInfoReq req;
//     IM::GetMultiUserInfoRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.add_users_id("99457c86e6f50001");
//     req.add_users_id("d77d41b174e70004");
//     req.add_users_id("1119c72d45aa0005");

//     stub.GetMultiUserInfo(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());

//     auto users_map = rsp.mutable_users_info();
//     IM::UserInfo zsr = (*users_map)["99457c86e6f50001"];
//     ASSERT_EQ(zsr.nickname(), "周树人");
//     ASSERT_EQ(zsr.description(), "呐喊, 朝花夕拾, 彷徨, 野草");
//     ASSERT_EQ(zsr.avatar(), "周树人头像");

//     IM::UserInfo ls = (*users_map)["d77d41b174e70004"];
//     ASSERT_EQ(zsr.nickname(), "老舍");
//     ASSERT_EQ(zsr.description(), "");

//     IM::UserInfo bj = (*users_map)["1119c72d45aa0005"];
//     ASSERT_EQ(zsr.nickname(), "巴金");
//     ASSERT_EQ(zsr.description(), "");
// }

// std::string code_id;
// void get_code(std::string phone) 
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     IM::PhoneVerifyCodeReq req;
//     req.set_request_id(IM::UUID::uuid());
//     req.set_phone_number(phone);
//     IM::PhoneVerifyCodeRsp rsp;
//     brpc::Controller cntl;
//     IM::UserService_Stub stub(channel.get());
//     stub.GetPhoneVerifyCode(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
//     code_id = rsp.verify_code_id();
// }

// 手机号注册
// TEST(user_test, phone_regist)
// {
//     auto channel = channel_manager->choose(FLAGS_user_service);
//     ASSERT_TRUE(channel);

//     get_code("aaabbbbcccc");
//     IM::UserService_Stub stub(channel.get());
//     IM::PhoneRegisterReq req;
//     IM::PhoneRegisterRsp rsp;
//     brpc::Controller* cntl = new brpc::Controller();

//     req.set_request_id(IM::UUID::uuid());
//     req.set_phone_number("aaabbbbcccc");
//     req.set_verify_code_id(code_id);
//     std::cout << "输入短信验证码:";
//     std::string code;
//     std::cin >> code;
//     req.set_verify_code(code);

//     stub.PhoneRegister(cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl->Failed());
//     ASSERT_TRUE(rsp.success());
// }

// 手机号登录
// TEST(user_test, phone_login)
// {
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     get_code("aaabbbbcccc");
//     auto channel = channel_manager->choose(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     IM::PhoneLoginReq req;
//     req.set_request_id(IM::UUID::uuid());
//     req.set_phone_number("aaabbbbcccc");
//     req.set_verify_code_id(code_id);
//     std::cout << "输入短信验证码:";
//     std::string code;
//     std::cin >> code;
//     req.set_verify_code(code);
//     IM::PhoneLoginRsp rsp;
//     brpc::Controller cntl;
//     IM::UserService_Stub stub(channel.get());
//     stub.PhoneLogin(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }

// // 手机号设置
// TEST(user_test, phone_set)
// {
//     get_code("aaabbbbcccc");
//     auto channel = channel_manager->choose(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     IM::SetUserPhoneNumberReq req;
//     req.set_request_id(IM::UUID::uuid());
//     req.set_user_id("99457c86e6f50001");

//     req.set_phone_number("aaabbbbcccc");
//     req.set_phone_verify_code_id(code_id);
//     std::cout << "输入验证码:";
//     std::string code;
//     std::cin >> code;
//     req.set_phone_verify_code(code);

//     IM::SetUserPhoneNumberRsp rsp;
//     brpc::Controller cntl;
//     IM::UserService_Stub stub(channel.get());
//     stub.SetUserPhoneNumber(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    testing::InitGoogleTest(&argc, argv);

    // 构造rpc信道管理对象
    channel_manager = std::make_shared<IM::ServiceManager>();
    channel_manager->declared(FLAGS_user_service);
    auto put_cb = std::bind(&IM::ServiceManager::onServiceOnline, channel_manager.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&IM::ServiceManager::onServiceOffline, channel_manager.get(), std::placeholders::_1, std::placeholders::_2);

    // 构造服务发现对象
    IM::Discovery::ptr dclient = std::make_shared<IM::Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);
    // user_info.set_nickname("鲁迅");
    // user_info.set_user_id("99457c86e6f50001");
    // user_info.set_description("呐喊, 朝花夕拾, 彷徨, 野草");
    // user_info.set_phone("aaabbbbcccc");
    // user_info.set_avatar("周树人头像");
    
    return RUN_ALL_TESTS();
}