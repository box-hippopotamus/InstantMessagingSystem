#pragma once

#include "etcd.hpp"
#include "logger.hpp"
#include "channel.hpp"
#include "redis_user.hpp"
#include "websocket_connection.hpp"

#include "user.pb.h"
#include "base.pb.h"
#include "file.pb.h"
#include "friend.pb.h"
#include "gateway.pb.h"
#include "message.pb.h"
#include "speech.pb.h"
#include "transmit.pb.h"
#include "notify.pb.h"

#include "httplib.h"

namespace IM
{
    class GatewayServer 
    {
    private:
        inline static const std::string GET_PHONE_VERIFY_CODE = "/service/user/get_phone_verify_code";
        inline static const std::string USERNAME_REGISTER     = "/service/user/username_register";
        inline static const std::string USERNAME_LOGIN        = "/service/user/username_login";
        inline static const std::string PHONE_REGISTER        = "/service/user/phone_register";
        inline static const std::string PHONE_LOGIN           = "/service/user/phone_login";
        inline static const std::string GET_USERINFO          = "/service/user/get_user_info";
        inline static const std::string SET_USER_AVATAR       = "/service/user/set_avatar";
        inline static const std::string SET_USER_NICKNAME     = "/service/user/set_nickname";
        inline static const std::string SET_USER_DESC         = "/service/user/set_description";
        inline static const std::string SET_USER_PHONE        = "/service/user/set_phone";
        inline static const std::string FRIEND_GET_LIST       = "/service/friend/get_friend_list";
        inline static const std::string FRIEND_APPLY          = "/service/friend/add_friend_apply";
        inline static const std::string FRIEND_APPLY_PROCESS  = "/service/friend/add_friend_process";
        inline static const std::string FRIEND_REMOVE         = "/service/friend/remove_friend";
        inline static const std::string FRIEND_SEARCH         = "/service/friend/search_friend";
        inline static const std::string FRIEND_GET_PENDING_EV = "/service/friend/get_pending_friend_events";
        inline static const std::string CSS_GET_LIST          = "/service/friend/get_chat_session_list";
        inline static const std::string CSS_CREATE            = "/service/friend/create_chat_session";
        inline static const std::string CSS_GET_MEMBER        = "/service/friend/get_chat_session_member";

        inline static const std::string CSS_ADD_MEMBER        = "/service/friend/group_add_member";
        inline static const std::string CSS_MODIFY_NAME       = "/service/friend/group_modify_name";
        inline static const std::string CSS_DEL_MEMBER        = "/service/friend/member_leave_group";

        inline static const std::string MSG_GET_RANGE         = "/service/message_storage/get_history";
        inline static const std::string MSG_GET_RECENT        = "/service/message_storage/get_recent";
        inline static const std::string MSG_KEY_SEARCH        = "/service/message_storage/search_history";
        inline static const std::string NEW_MESSAGE           = "/service/message_transmit/new_message";
        inline static const std::string FILE_GET_SINGLE       = "/service/file/get_single_file";
        inline static const std::string FILE_GET_MULTI        = "/service/file/get_multi_file";
        inline static const std::string FILE_PUT_SINGLE       = "/service/file/put_single_file";
        inline static const std::string FILE_PUT_MULTI        = "/service/file/put_multi_file";
        inline static const std::string SPEECH_RECOGNITION    = "/service/speech/recognition";

        // 路由表
        using RouteHandler = std::function<void(const httplib::Request&, httplib::Response&)>;

        std::unordered_map<std::string, RouteHandler> _route_table = 
        {
            // 用户服务
            { GET_PHONE_VERIFY_CODE, [this](const httplib::Request& req, httplib::Response& res) { GetPhoneVerifyCode(req, res); }},
            { USERNAME_REGISTER,     [this](const httplib::Request& req, httplib::Response& res) { UserRegister(req, res); }},
            { USERNAME_LOGIN,        [this](const httplib::Request& req, httplib::Response& res) { UserLogin(req, res); }},
            { PHONE_REGISTER,        [this](const httplib::Request& req, httplib::Response& res) { PhoneRegister(req, res); }},
            { PHONE_LOGIN,           [this](const httplib::Request& req, httplib::Response& res) { PhoneLogin(req, res); }},
            { GET_USERINFO,          [this](const httplib::Request& req, httplib::Response& res) { GetUserInfo(req, res); }},
            { SET_USER_AVATAR,       [this](const httplib::Request& req, httplib::Response& res) { SetUserAvatar(req, res); }},
            { SET_USER_NICKNAME,     [this](const httplib::Request& req, httplib::Response& res) { SetUserNickname(req, res); }},
            { SET_USER_DESC,         [this](const httplib::Request& req, httplib::Response& res) { SetUserDescription(req, res); }},
            { SET_USER_PHONE,        [this](const httplib::Request& req, httplib::Response& res) { SetUserPhoneNumber(req, res); }},
            // 好友服务
            { FRIEND_GET_LIST,       [this](const httplib::Request& req, httplib::Response& res) { GetFriendList(req, res); }},
            { FRIEND_APPLY,          [this](const httplib::Request& req, httplib::Response& res) { FriendAdd(req, res); }},
            { FRIEND_APPLY_PROCESS,  [this](const httplib::Request& req, httplib::Response& res) { FriendAddProcess(req, res); }},
            { FRIEND_REMOVE,         [this](const httplib::Request& req, httplib::Response& res) { FriendRemove(req, res); }},
            { FRIEND_SEARCH,         [this](const httplib::Request& req, httplib::Response& res) { FriendSearch(req, res); }},
            { FRIEND_GET_PENDING_EV, [this](const httplib::Request& req, httplib::Response& res) { GetPendingFriendEventList(req, res); }},
            { CSS_GET_LIST,          [this](const httplib::Request& req, httplib::Response& res) { GetChatSessionList(req, res); }},
            { CSS_CREATE,            [this](const httplib::Request& req, httplib::Response& res) { ChatSessionCreate(req, res); }},
            { CSS_GET_MEMBER,        [this](const httplib::Request& req, httplib::Response& res) { GetChatSessionMember(req, res); }},
            { CSS_ADD_MEMBER,        [this](const httplib::Request& req, httplib::Response& res) { GroupAddMember(req, res); }},
            { CSS_MODIFY_NAME,       [this](const httplib::Request& req, httplib::Response& res) { GroupModifyName(req, res); }},
            { CSS_DEL_MEMBER,        [this](const httplib::Request& req, httplib::Response& res) { MemberLeaveGroup(req, res); }},
            // 消息存储服务
            { MSG_GET_RANGE,         [this](const httplib::Request& req, httplib::Response& res) { GetHistoryMsg(req, res); }},
            { MSG_GET_RECENT,        [this](const httplib::Request& req, httplib::Response& res) { GetRecentMsg(req, res); }},
            { MSG_KEY_SEARCH,        [this](const httplib::Request& req, httplib::Response& res) { MsgSearch(req, res); }},
            // 消息转发服务
            { NEW_MESSAGE,           [this](const httplib::Request& req, httplib::Response& res) { NewMessage(req, res); }},
            // 文件存储服务
            { FILE_GET_SINGLE,       [this](const httplib::Request& req, httplib::Response& res) { GetSingleFile(req, res); }},
            { FILE_GET_MULTI,        [this](const httplib::Request& req, httplib::Response& res) { GetMultiFile(req, res); }},
            { FILE_PUT_SINGLE,       [this](const httplib::Request& req, httplib::Response& res) { PutSingleFile(req, res); }},
            { FILE_PUT_MULTI,        [this](const httplib::Request& req, httplib::Response& res) { PutMultiFile(req, res); }},
            // 语音服务
            { SPEECH_RECOGNITION,    [this](const httplib::Request& req, httplib::Response& res) { SpeechRecognition(req, res); }},
        };

    public:
        using ptr = std::shared_ptr<GatewayServer>;

        GatewayServer(int websocket_port, int http_port,
                        const std::shared_ptr<sw::redis::Redis>& redis_client,
                        const ServiceManager::ptr& channel_manager,
                        const Discovery::ptr& dis_client,
                        const std::string user_service_name,
                        const std::string file_service_name,
                        const std::string speech_service_name,
                        const std::string message_service_name,
                        const std::string transmit_service_name,
                        const std::string friend_service_name)
            : _redis_session(std::make_shared<Session>(redis_client))
            , _redis_status(std::make_shared<Status>(redis_client))
            , _channel_manager(channel_manager)
            , _dis_client(dis_client)
            , _user_service_name(user_service_name)
            , _file_service_name(file_service_name)
            , _speech_service_name(speech_service_name)
            , _message_service_name(message_service_name)
            , _transmit_service_name(transmit_service_name)
            , _friend_service_name(friend_service_name)
            , _connections(std::make_shared<Connection>())
        {
            // ========== websocket ==========
            _ws_server.set_access_channels(websocketpp::log::alevel::none); // 关闭日志输出
            _ws_server.init_asio(); // 初始化asio

            // 设置回调
            auto open_handler = [this](websocketpp::connection_hdl hdl){
                onOpen(hdl);
            };

            auto close_handler = [this](websocketpp::connection_hdl hdl){
                onClose(hdl);
            };

            auto msg_handler = [this](websocketpp::connection_hdl hdl, Connection::server_t::message_ptr msg){
                onMessage(hdl, msg);
            };

            _ws_server.set_open_handler(open_handler);
            _ws_server.set_close_handler(close_handler);
            _ws_server.set_message_handler(msg_handler); // 消息回调
            _ws_server.set_reuse_addr(true); // 启用地址复用
            _ws_server.listen(websocket_port); // 端口
            _ws_server.start_accept(); // 开始监听

            // ========== httplib ==========
            // 注册 HTTP 路由
            for (const auto& [route, handler] : _route_table)
                _http_server.Post(route, handler);
            
            // 创建线程跑 httplib
            _http_thread = std::thread([this, http_port](){
                _http_server.listen("0.0.0.0", http_port);
            });

            _http_thread.detach();
        }

        void start() 
        {
            _ws_server.run(); // 启动服务
        }

    private:
        void keepAlive(Connection::server_t::connection_ptr conn) 
        {
            if (!conn || conn->get_state() != websocketpp::session::state::value::open)
            {
                LOG_DEBUG("连接状态异常，结束连接保活!");
                return;
            }

            conn->ping("");
            _ws_server.set_timer(60000,  [this, conn](const std::error_code& ec) {
                if (!ec) keepAlive(conn);
            });
        }

        // 连接建立回调
        void onOpen(websocketpp::connection_hdl hdl)
        {
            LOG_DEBUG("websocket 连接建立成功!");
        }

        // 连接关闭回调
        void onClose(websocketpp::connection_hdl hdl)
        {
            // 获取连接对象
            auto conn = _ws_server.get_con_from_hdl(hdl);
            std::string uid, ssid;
            if (!_connections->client(conn, uid, ssid))
            {
                LOG_WARN("未找到连接相关信息!");
                return;
            }

            _redis_session->remove(ssid);
            _redis_status->remove(uid);
            _connections->remove(conn);
        }

        // 消息到达回调
        void onMessage(websocketpp::connection_hdl hdl, Connection::server_t::message_ptr msg)
        {
            auto conn = _ws_server.get_con_from_hdl(hdl);

            // 反序列化
            ClientAuthenticationReq request;
            if (!request.ParseFromString(msg->get_payload()))
            {
                LOG_ERROR("反序列化失败!");
                return _ws_server.close(hdl, websocketpp::close::status::unsupported_data, "反序列化失败!");
            }

            // redis 查找会话信息 (用户登录时添加到redis)
            std::string ssid = request.session_id();
            auto uid = _redis_session->uid(ssid);
            if (!uid)
            {
                LOG_ERROR("用户没有对应的会话信息!");
                return _ws_server.close(hdl, websocketpp::close::status::unsupported_data, "用户没有对应的会话信息!");
            }

            // 添加长连接
            _connections->insert(conn, *uid, ssid);
            keepAlive(conn);
        }
        
        template <typename rpc_req, typename rpc_rsp, typename rpc_stub>
        using rpc_call_t = std::function<void(rpc_stub*, brpc::Controller*, rpc_req*, rpc_rsp*, google::protobuf::Closure*)>;
        
        template <typename rpc_req, typename rpc_rsp, typename rpc_stub, bool check_session>
        void RpcCaller(const httplib::Request& request, 
                        httplib::Response& response, 
                        const std::string& service_name, 
                        std::string rpc_name, 
                        rpc_call_t<rpc_req, rpc_rsp, rpc_stub> rpc_call)
        {
            LOG_DEBUG("客户端请求: {} {}", service_name, rpc_name);

            // 反序列化请求
            rpc_req req;
            rpc_rsp rsp;
            brpc::Controller cntl;
            rsp.set_success(true);

            auto err_response = [&req, &rsp, &response](const std::string &errmsg) {
                rsp.set_success(false);
                rsp.set_errmsg(errmsg);
                response.set_content(rsp.SerializeAsString(), "application/x-protbuf");
            };

            auto channel = _channel_manager->choose(service_name);
            if (!channel) 
            {
                LOG_ERROR("{}: 无节点可提供 {} 服务!", req.request_id(), service_name);
                return err_response("无节点可提供相关服务!");
            }

            if (!req.ParseFromString(request.body)) 
            {
                LOG_ERROR("{}: {} 请求正文反序列化失败!", service_name, rpc_name);
                return err_response("请求正文反序列化失败!");
            }

            // 鉴权
            if constexpr (check_session) // 必须在编译期确认，否则 req 可能没有 session_id 函数
            {
                std::string ssid = req.session_id();
                auto uid = _redis_session->uid(ssid);
                if (!uid) 
                {
                    LOG_ERROR("{}: {} 获取用户信息失败!", rpc_name, ssid);
                    return err_response("session无效!");
                }
                req.set_user_id(*uid);
            }

            // 转发请求
            rpc_stub stub(channel.get());
            rpc_call(&stub, &cntl, &req, &rsp, nullptr);
            if (cntl.Failed()) 
            {
                LOG_ERROR("{}: {}服务调用失败!", service_name, rpc_name);
                return err_response("子服务调用失败!");
            }

            // 序列化响应
            response.set_content(rsp.SerializeAsString(), "application/x-protbuf");
        }

        // ==================================== 用户服务 ====================================
        void GetPhoneVerifyCode(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<PhoneVerifyCodeReq, PhoneVerifyCodeRsp, IM::UserService_Stub, false>(
                request, response, _user_service_name, "GetPhoneVerifyCode 获取短信验证码",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, PhoneVerifyCodeReq* req, 
                    PhoneVerifyCodeRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetPhoneVerifyCode(cntl, req, rsp, done);
                }
            );
        }

        void UserRegister(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<UserRegisterReq, UserRegisterRsp, IM::UserService_Stub, false>(
                request, response, _user_service_name, "UserRegister 用户注册",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, UserRegisterReq* req, 
                    UserRegisterRsp* rsp, google::protobuf::Closure* done) {
                    stub->UserRegister(cntl, req, rsp, done);
                }
            );
        }

        void UserLogin(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<UserLoginReq, UserLoginRsp, IM::UserService_Stub, false>(
                request, response, _user_service_name, "UserLogin 用户登录",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, UserLoginReq* req, 
                    UserLoginRsp* rsp, google::protobuf::Closure* done) {
                    stub->UserLogin(cntl, req, rsp, done);
                }
            );
        }

        void PhoneRegister(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<PhoneRegisterReq, PhoneRegisterRsp, IM::UserService_Stub, false>(
                request, response, _user_service_name, "PhoneRegister 手机注册",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, PhoneRegisterReq* req, 
                    PhoneRegisterRsp* rsp, google::protobuf::Closure* done) {
                    stub->PhoneRegister(cntl, req, rsp, done);
                }
            );
        }

        void PhoneLogin(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<PhoneLoginReq, PhoneLoginRsp, IM::UserService_Stub, false>(
                request, response, _user_service_name, "PhoneLogin 手机登录",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, PhoneLoginReq* req, 
                    PhoneLoginRsp* rsp, google::protobuf::Closure* done) {
                    stub->PhoneLogin(cntl, req, rsp, done);
                }
            );
        }

        void GetUserInfo(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetUserInfoReq, GetUserInfoRsp, IM::UserService_Stub, true>(
                request, response, _user_service_name, "GetUserInfo 获取用户信息",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, GetUserInfoReq* req, 
                    GetUserInfoRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetUserInfo(cntl, req, rsp, done);
                }
            );
        }

        void SetUserAvatar(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<SetUserAvatarReq, SetUserAvatarRsp, IM::UserService_Stub, true>(
                request, response, _user_service_name, "SetUserAvatar 设置用户头像",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, SetUserAvatarReq* req, 
                    SetUserAvatarRsp* rsp, google::protobuf::Closure* done) {
                    stub->SetUserAvatar(cntl, req, rsp, done);
                }
            );
        }

        void SetUserNickname(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<SetUserNicknameReq, SetUserNicknameRsp, IM::UserService_Stub, true>(
                request, response, _user_service_name, "SetUserNickname 设置用户名",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, SetUserNicknameReq* req, 
                    SetUserNicknameRsp* rsp, google::protobuf::Closure* done) {
                    stub->SetUserNickname(cntl, req, rsp, done);
                }
            );
        }

        void SetUserDescription(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<SetUserDescriptionReq, SetUserDescriptionRsp, IM::UserService_Stub, true>(
                request, response, _user_service_name, "SetUserDescription 设置用户签名",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, SetUserDescriptionReq* req, 
                    SetUserDescriptionRsp* rsp, google::protobuf::Closure* done) {
                    stub->SetUserDescription(cntl, req, rsp, done);
                }
            );
        }

        void SetUserPhoneNumber(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<SetUserPhoneNumberReq, SetUserPhoneNumberRsp, IM::UserService_Stub, true>(
                request, response, _user_service_name, "SetUserPhoneNumber 设置用户电话",
                [](IM::UserService_Stub* stub, brpc::Controller* cntl, SetUserPhoneNumberReq* req, 
                    SetUserPhoneNumberRsp* rsp, google::protobuf::Closure* done) {
                    stub->SetUserPhoneNumber(cntl, req, rsp, done);
                }
            );
        }

        // ==================================== 好友服务 ====================================
        void GetFriendList(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetFriendListReq, GetFriendListRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "GetFriendList 获取好友列表",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, GetFriendListReq* req, 
                    GetFriendListRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetFriendList(cntl, req, rsp, done);
                }
            );
        }

        void FriendAdd(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<FriendAddReq, FriendAddRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "FriendAdd 添加好友",
                [this](IM::FriendService_Stub* stub, brpc::Controller* cntl, FriendAddReq* req, 
                    FriendAddRsp* rsp, google::protobuf::Closure* done) 
                    { 
                        stub->FriendAdd(cntl, req, rsp, done); 
                        if (cntl->Failed() || !rsp->success())
                            return;

                        // 推送
                        auto conn = _connections->connection(req->respondent_id());
                        if (!conn) 
                            return;

                        auto user_rsp = _GetUserInfo(req->request_id(), req->user_id());
                        if (!user_rsp) 
                        {
                            LOG_ERROR("{} 获取客户端用户信息失败!", req->request_id());
                            rsp->set_errmsg("获取客户端用户信息失败!");
                            rsp->set_success(false);
                            return;
                        }
                        
                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::FRIEND_ADD_APPLY_NOTIFY);
                        notify.mutable_friend_add_apply()->mutable_user_info()->CopyFrom(user_rsp->user_info());
                        conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    });
        }

        void FriendAddProcess(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<FriendAddProcessReq, FriendAddProcessRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "FriendAddProcess 处理好友申请",
                [this](IM::FriendService_Stub* stub, brpc::Controller* cntl, FriendAddProcessReq* req, 
                    FriendAddProcessRsp* rsp, google::protobuf::Closure* done)
                {
                    stub->FriendAddProcess(cntl, req, rsp, done);

                    if (cntl->Failed() || !rsp->success())
                        return;
                        
                    // 被申请人id
                    auto process_user_rsp = _GetUserInfo(req->request_id(), req->user_id());
                    // 申请人id
                    auto apply_user_rsp = _GetUserInfo(req->request_id(), req->apply_user_id());
                    if (!process_user_rsp || !apply_user_rsp) 
                    {
                        LOG_ERROR("{} 获取用户信息失败!", req->request_id());
                        rsp->set_errmsg("获取用户信息失败!");
                        rsp->set_success(false);
                        return;
                    }

                    // 下线则不通知
                    auto process_conn = _connections->connection(req->user_id());
                    if (!process_conn) 
                        LOG_DEBUG("未找到被申请人的长连接!");

                    auto apply_conn = _connections->connection(req->apply_user_id());
                    if (!apply_conn) 
                        LOG_DEBUG("未找到申请人的长连接!");

                    // 通知申请人处理结果
                    if (apply_conn) 
                    {
                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::FRIEND_ADD_PROCESS_NOTIFY);
                        auto process_result = notify.mutable_friend_process_result();
                        process_result->mutable_user_info()->CopyFrom(process_user_rsp->user_info());
                        process_result->set_agree(req->agree());
                        apply_conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    }

                    // 同意好友请求,通知双方单聊会话的创建
                    // 通知申请人
                    if (req->agree() && apply_conn) 
                    {
                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::CHAT_SESSION_CREATE_NOTIFY);
                        auto chat_session_info = notify.mutable_new_chat_session_info()->mutable_chat_session_info();
                        chat_session_info->set_single_chat_friend_id(req->user_id());
                        chat_session_info->set_chat_session_id(rsp->new_session_id());
                        chat_session_info->set_chat_session_name(process_user_rsp->user_info().nickname());
                        chat_session_info->set_avatar(process_user_rsp->user_info().avatar());

                        apply_conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    }

                    // 通知被申请人
                    if (req->agree() && process_conn) 
                    { 
                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::CHAT_SESSION_CREATE_NOTIFY);
                        auto chat_session_info = notify.mutable_new_chat_session_info()->mutable_chat_session_info();
                        chat_session_info->set_single_chat_friend_id(req->apply_user_id());
                        chat_session_info->set_chat_session_id(rsp->new_session_id());
                        chat_session_info->set_chat_session_name(apply_user_rsp->user_info().nickname());
                        chat_session_info->set_avatar(apply_user_rsp->user_info().avatar());

                        process_conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    }
                }
            );
        }

        void FriendRemove(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<FriendRemoveReq, FriendRemoveRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "FriendRemove 删除好友",
                [this](IM::FriendService_Stub* stub, brpc::Controller* cntl, FriendRemoveReq* req, 
                    FriendRemoveRsp* rsp, google::protobuf::Closure* done) 
                {
                    stub->FriendRemove(cntl, req, rsp, done);

                    if (cntl->Failed() || !rsp->success())
                        return;

                    // 推送
                    auto conn = _connections->connection(req->peer_id());
                    if (!conn) 
                        return;

                    NotifyMessage notify;
                    notify.set_notify_type(NotifyType::FRIEND_REMOVE_NOTIFY);
                    notify.mutable_friend_remove()->set_user_id(req->user_id());
                    conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                }
            );
        }

        void FriendSearch(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<FriendSearchReq, FriendSearchRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "FriendSearch 搜索好友",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, FriendSearchReq* req, 
                    FriendSearchRsp* rsp, google::protobuf::Closure* done) {
                    stub->FriendSearch(cntl, req, rsp, done);
                }
            );
        }

        void GetPendingFriendEventList(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetPendingFriendEventListReq, GetPendingFriendEventListRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "GetPendingFriendEventList 获取好友申请列表",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, GetPendingFriendEventListReq* req, 
                    GetPendingFriendEventListRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetPendingFriendEventList(cntl, req, rsp, done);
                }
            );
        }

        void GetChatSessionList(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetChatSessionListReq, GetChatSessionListRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "GetChatSessionList 获取会话列表",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, GetChatSessionListReq* req, 
                    GetChatSessionListRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetChatSessionList(cntl, req, rsp, done);
                }
            );
        }

        void ChatSessionCreate(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<ChatSessionCreateReq, ChatSessionCreateRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "ChatSessionCreate 创建会话",
                [this](IM::FriendService_Stub* stub, brpc::Controller* cntl, ChatSessionCreateReq* req, 
                    ChatSessionCreateRsp* rsp, google::protobuf::Closure* done) 
                {
                    stub->ChatSessionCreate(cntl, req, rsp, done);

                    if (cntl->Failed() || !rsp->success())
                        return;

                    // 推送
                    for (int i = 0; i < req->member_id_list_size(); i++)
                    {
                        auto conn = _connections->connection(req->member_id_list(i));
                        if (!conn) 
                            continue;

                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::CHAT_SESSION_CREATE_NOTIFY);
                        auto chat_session = notify.mutable_new_chat_session_info();
                        chat_session->mutable_chat_session_info()->CopyFrom(rsp->chat_session_info());
                        conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    }

                    rsp->clear_chat_session_info(); // 该数据已通过 websocket 推送
                }
            );
        }

        void GetChatSessionMember(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetChatSessionMemberReq, GetChatSessionMemberRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "GetChatSessionMember 获取会话成员",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, GetChatSessionMemberReq* req, 
                    GetChatSessionMemberRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetChatSessionMember(cntl, req, rsp, done);
                }
            );
        }

        void GroupAddMember(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GroupAddMemberReq, GroupAddMemberRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "GroupAddMember 添加会话成员",
                [this](IM::FriendService_Stub* stub, brpc::Controller* cntl, GroupAddMemberReq* req, 
                    GroupAddMemberRsp* rsp, google::protobuf::Closure* done) {
                    stub->GroupAddMember(cntl, req, rsp, done);
                    
                    if (cntl->Failed() || !rsp->success())
                        return;

                    // 推送
                    for (int i = 0; i < req->member_id_list_size(); i++)
                    {
                        auto conn = _connections->connection(req->member_id_list(i));
                        if (!conn) 
                            continue;

                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::CHAT_SESSION_CREATE_NOTIFY);
                        auto chat_session = notify.mutable_new_chat_session_info();
                        chat_session->mutable_chat_session_info()->CopyFrom(rsp->chat_session_info());
                        conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    }

                    rsp->clear_chat_session_info(); // 该数据已通过 websocket 推送
                }
            );
        }

        void GroupModifyName(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GroupModifyNameReq, GroupModifyNameRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "GroupModifyName 修改会话名称",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, GroupModifyNameReq* req, 
                    GroupModifyNameRsp* rsp, google::protobuf::Closure* done) {
                    stub->GroupModifyName(cntl, req, rsp, done);
                }
            );
        }

        void MemberLeaveGroup(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<MemberLeaveGroupReq, MemberLeaveGroupRsp, IM::FriendService_Stub, true>(
                request, response, _friend_service_name, "MemberLeaveGroup 成员退出会话",
                [](IM::FriendService_Stub* stub, brpc::Controller* cntl, MemberLeaveGroupReq* req, 
                    MemberLeaveGroupRsp* rsp, google::protobuf::Closure* done) {
                    stub->MemberLeaveGroup(cntl, req, rsp, done);
                }
            );
        }

        // ==================================== 消息存储服务 ====================================
        void GetHistoryMsg(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetHistoryMsgReq, GetHistoryMsgRsp, IM::MsgStorageService_Stub, true>(
                request, response, _message_service_name, "GetHistoryMsg 获取历史消息",
                [](IM::MsgStorageService_Stub* stub, brpc::Controller* cntl, GetHistoryMsgReq* req, 
                    GetHistoryMsgRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetHistoryMsg(cntl, req, rsp, done);
                }
            );
        }

        void GetRecentMsg(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetRecentMsgReq, GetRecentMsgRsp, IM::MsgStorageService_Stub, true>(
                request, response, _message_service_name, "GetRecentMsg 获取最近消息",
                [](IM::MsgStorageService_Stub* stub, brpc::Controller* cntl, GetRecentMsgReq* req, 
                    GetRecentMsgRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetRecentMsg(cntl, req, rsp, done);
                }
            );
        }

        void MsgSearch(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<MsgSearchReq, MsgSearchRsp, IM::MsgStorageService_Stub, true>(
                request, response, _message_service_name, "MsgSearch 搜索消息",
                [](IM::MsgStorageService_Stub* stub, brpc::Controller* cntl, MsgSearchReq* req, 
                    MsgSearchRsp* rsp, google::protobuf::Closure* done) {
                    stub->MsgSearch(cntl, req, rsp, done);
                }
            );
        }

        // ==================================== 文件服务 ====================================
        void GetSingleFile(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetSingleFileReq, GetSingleFileRsp, IM::FileService_Stub, true>(
                request, response, _file_service_name, "GetSingleFile 获取单文件",
                [](IM::FileService_Stub* stub, brpc::Controller* cntl, GetSingleFileReq* req, 
                    GetSingleFileRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetSingleFile(cntl, req, rsp, done);
                }
            );
        }

        void GetMultiFile(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<GetMultiFileReq, GetMultiFileRsp, IM::FileService_Stub, true>(
                request, response, _file_service_name, "GetMultiFile 获取多文件",
                [](IM::FileService_Stub* stub, brpc::Controller* cntl, GetMultiFileReq* req, 
                    GetMultiFileRsp* rsp, google::protobuf::Closure* done) {
                    stub->GetMultiFile(cntl, req, rsp, done);
                }
            );
        }

        void PutSingleFile(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<PutSingleFileReq, PutSingleFileRsp, IM::FileService_Stub, true>(
                request, response, _file_service_name, "PutSingleFile 上传单文件",
                [](IM::FileService_Stub* stub, brpc::Controller* cntl, PutSingleFileReq* req, 
                    PutSingleFileRsp* rsp, google::protobuf::Closure* done) {
                    stub->PutSingleFile(cntl, req, rsp, done);
                }
            );
        }

        void PutMultiFile(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<PutMultiFileReq, PutMultiFileRsp, IM::FileService_Stub, true>(
                request, response, _file_service_name, "PutMultiFile 上传多文件",
                [](IM::FileService_Stub* stub, brpc::Controller* cntl, PutMultiFileReq* req, 
                    PutMultiFileRsp* rsp, google::protobuf::Closure* done) {
                    stub->PutMultiFile(cntl, req, rsp, done);
                }
            );
        }

        // ==================================== 语音服务 ====================================
        void SpeechRecognition(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<SpeechRecognitionReq, SpeechRecognitionRsp, IM::SpeechService_Stub, true>(
                request, response, _speech_service_name, "SpeechRecognition 语音识别",
                [](IM::SpeechService_Stub* stub, brpc::Controller* cntl, SpeechRecognitionReq* req, 
                    SpeechRecognitionRsp* rsp, google::protobuf::Closure* done) {
                    stub->SpeechRecognition(cntl, req, rsp, done);
                }
            );
        }

        // ==================================== 消息转发服务 ====================================
        void NewMessage(const httplib::Request& request, httplib::Response& response)
        {
            RpcCaller<NewMessageReq, NewMessageRsp, IM::MsgTransmitService_Stub, true>(
                request, response, _transmit_service_name, "MsgTransmit 消息转发",
                [this](IM::MsgTransmitService_Stub* stub, brpc::Controller* cntl, NewMessageReq* req, 
                    NewMessageRsp* rsp, google::protobuf::Closure* done) 
                {
                    GetTransmitTargetRsp target_rsp;
                    stub->GetTransmitTarget(cntl, req, &target_rsp, done);
                    
                    if (cntl->Failed())
                        return;
                                            
                    if (!target_rsp.success())
                    {
                        rsp->set_success(target_rsp.success());
                        rsp->set_errmsg(target_rsp.errmsg());
                        return;
                    }

                    // 推送        
                    for (int i = 0; i < target_rsp.target_id_list_size(); i++) 
                    {
                        std::string notify_uid = target_rsp.target_id_list(i);
                        if (notify_uid == req->user_id()) //不通知自己
                            continue; 

                        auto conn = _connections->connection(notify_uid);
                        if (!conn) 
                            continue;

                        NotifyMessage notify;
                        notify.set_notify_type(NotifyType::CHAT_MESSAGE_NOTIFY);
                        auto msg_info = notify.mutable_new_message_info();
                        msg_info->mutable_message_info()->CopyFrom(target_rsp.message());

                        conn->send(notify.SerializeAsString(), websocketpp::frame::opcode::value::binary);
                    }

                    rsp->set_request_id(req->request_id());
                }
            );
        }

    private:
        std::shared_ptr<GetUserInfoRsp> _GetUserInfo(const std::string& rid, const std::string& uid) 
        {
            GetUserInfoReq req;
            auto rsp = std::make_shared<GetUserInfoRsp>();
            req.set_request_id(rid);
            req.set_user_id(uid);

            // 请求转发
            auto channel = _channel_manager->choose(_user_service_name);
            if (!channel) 
            {
                LOG_ERROR("{} 没有可提供业务处理的服务节点!", req.request_id());
                return std::shared_ptr<GetUserInfoRsp>();
            }

            IM::UserService_Stub stub(channel.get());
            brpc::Controller cntl;
            stub.GetUserInfo(&cntl, &req, rsp.get(), nullptr);
            if (cntl.Failed())
             {
                LOG_ERROR("{} 用户子服务调用失败!", req.request_id());
                return std::shared_ptr<GetUserInfoRsp>();
            }

            return rsp;
        }

    private:
        Session::ptr _redis_session;
        Status::ptr _redis_status;

        std::string _user_service_name;
        std::string _file_service_name;
        std::string _speech_service_name;
        std::string _message_service_name;
        std::string _transmit_service_name;
        std::string _friend_service_name;
        ServiceManager::ptr _channel_manager;
        Discovery::ptr _dis_client;

        Connection::ptr _connections;

        Connection::server_t _ws_server; 
        httplib::Server _http_server;
        std::thread _http_thread; // 给 httplib 额外的线程，防止与 websocket 冲突
    };

    class GatewayServerBuilder 
    {
        public:
            // redis
            void init_redis_client(const std::string &host, int port, int db, bool keep_alive) 
            {
                _redis_client = RedisClientFactory::create(host, port, db, keep_alive);
            }

            // discover
            void init_discovery_client(const std::string &reg_host,
                                        const std::string &base_service_name,
                                        const std::string &file_service_name,
                                        const std::string &speech_service_name,
                                        const std::string &message_service_name,
                                        const std::string &friend_service_name,
                                        const std::string &user_service_name,
                                        const std::string &transmit_service_name) 
            {
                _file_service_name      = file_service_name;
                _speech_service_name    = speech_service_name;
                _message_service_name   = message_service_name;
                _friend_service_name    = friend_service_name;
                _user_service_name      = user_service_name;
                _transmit_service_name = transmit_service_name;

                _channel_manager = std::make_shared<ServiceManager>();
                _channel_manager->declared(file_service_name);
                _channel_manager->declared(speech_service_name);
                _channel_manager->declared(message_service_name);
                _channel_manager->declared(friend_service_name);
                _channel_manager->declared(user_service_name);
                _channel_manager->declared(transmit_service_name);

                auto put_cb = [this](const std::string& service_instance, const std::string& host) {
                    _channel_manager->onServiceOnline(service_instance, host); 
                };
    
                auto del_cb = [this](const std::string& service_instance, const std::string& host) {
                    _channel_manager->onServiceOffline(service_instance, host); 
                };

                _dis_client = std::make_shared<Discovery>(reg_host, base_service_name, put_cb, del_cb);
            }

            void init_socket_server(int websocket_port, int http_port) 
            {
                _websocket_port = websocket_port;
                _http_port = http_port;
            }

            // rpc
            GatewayServer::ptr build() 
            {
                if (!_redis_client || !_dis_client || !_channel_manager) 
                {
                    LOG_ERROR("构造尚未完成!");
                    abort();
                }

                return std::make_shared<GatewayServer>(_websocket_port, _http_port, _redis_client, _channel_manager, 
                                                        _dis_client, _user_service_name, _file_service_name,
                                                        _speech_service_name, _message_service_name, 
                                                        _transmit_service_name, _friend_service_name);
            }

        private:
            int _websocket_port;
            int _http_port;

            std::shared_ptr<sw::redis::Redis> _redis_client;

            std::string _file_service_name;
            std::string _speech_service_name;
            std::string _message_service_name;
            std::string _friend_service_name;
            std::string _user_service_name;
            std::string _transmit_service_name;
            
            ServiceManager::ptr _channel_manager;
            Discovery::ptr _dis_client;
    };
}