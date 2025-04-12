#pragma once

#include <cctype>

#include <brpc/server.h>
#include <butil/logging.h>

#include "utils.hpp"
#include "etcd.hpp"
#include "logger.hpp"
#include "channel.hpp"
#include "mysql_session_member.hpp"
#include "rabbitmq.hpp"

#include "base.pb.h"
#include "user.pb.h"
#include "transmit.pb.h"

namespace IM
{
    // 用户服务RPC接口类
    class TransmitServiceImpl : public MsgTransmitService
    {
    public:
        using ptr = std::shared_ptr<TransmitServiceImpl>;

        TransmitServiceImpl(const std::string& user_service_name,
                            const ServiceManager::ptr& channel_manager,
                            const std::shared_ptr<odb::core::database>& mysql_client,
                            const MQClient::ptr& mq_client,
                            const std::string exchange_name,
                            const std::string bindkey)
            : _user_service_name(user_service_name)
            , _channel_manager(channel_manager)
            , _mysql_session_member(std::make_shared<SessionMemberTable>(mysql_client))
            , _mq_client(mq_client)
            , _exchange_name(exchange_name)
            , _bindkey(bindkey)
        {}

        ~TransmitServiceImpl() = default;

        void GetTransmitTarget(google::protobuf::RpcController* controller,
                                const ::IM::NewMessageReq* request,
                                ::IM::GetTransmitTargetRsp* response,
                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());
            
            auto err_response = [this, response](const std::string& errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 获取请求信息
            std::string uid = request->user_id();
            std::string ssid = request->session_id();
            std::string csid = request->chat_session_id();
            const MessageContent& content = request->message();

            // 从用户服务获取详细信息
            auto channel = _channel_manager->choose(_user_service_name);
            if (!channel)
            {
                LOG_ERROR("{}: 获取用户服务失败!", request->request_id());
                return err_response("获取用户服务失败");
            }

            UserService_Stub stub(channel.get());
            GetUserInfoReq req;
            GetUserInfoRsp rsp;
            brpc::Controller cntl;
            req.set_request_id(UUID::uuid());
            req.set_user_id(uid);
            req.set_session_id(ssid);
            stub.GetUserInfo(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || !rsp.success()) 
            {
                LOG_ERROR("{}: 用户子服务调用失败: {}", request->request_id(), cntl.ErrorText());
                return err_response("用户子服务调用失败!");
            }

            // 数据库查询对话成员
            auto suid_list = _mysql_session_member->members(csid);
            if (suid_list.empty())
            {
                LOG_ERROR("{}: 数据库查询错误!", request->request_id());
                return err_response("数据库查询错误!");
            }

            // 组织信息，序列化
            MessageInfo message;
            message.set_message_id(UUID::uuid());
            message.set_chat_session_id(csid);
            message.set_timestamp(time(nullptr));
            message.mutable_sender()->CopyFrom(rsp.user_info());
            message.mutable_message()->CopyFrom(content);

            // 向 mq 转发消息，持久化保存
            if (!_mq_client->publish(_exchange_name, message.SerializeAsString(), _bindkey)) 
            {
                LOG_ERROR("{}: 持久化消息失败: {}", request->request_id(), cntl.ErrorText());
                return err_response("持久化消息失败!");
            }

            // 响应
            for (auto& suid : suid_list)
                response->add_target_id_list(suid);

            response->mutable_message()->CopyFrom(message);
            response->set_success(true);
        }

    private:
        // 调用用户服务
        std::string _user_service_name;
        ServiceManager::ptr _channel_manager;

        SessionMemberTable::ptr _mysql_session_member;
        MQClient::ptr _mq_client;
        std::string _exchange_name;
        std::string _bindkey;
    };

    class TransmitServer
    {
    public:
        using ptr = std::shared_ptr<TransmitServer>;

        TransmitServer(const Discovery::ptr& dis_client,
                        const Registry::ptr& reg_client,
                        const std::shared_ptr<brpc::Server>& rpc_server,
                        const std::shared_ptr<odb::core::database>& mysql_client)
            : _dis_client(dis_client)
            , _reg_client(reg_client)
            , _rpc_server(rpc_server)
            , _mysql_client(mysql_client)
        {}

        // 启动rpc服务
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        Registry::ptr _reg_client;
        Discovery::ptr _dis_client;
        std::shared_ptr<brpc::Server> _rpc_server;
        std::shared_ptr<odb::core::database> _mysql_client;
    };
    
    class TransmitServiceBuilder
    {
    public:
        // 构造rabbitMQ
        void init_mq_client(const std::string& user,
                            const std::string& password,
                            const std::string& host,
                            const std::string& exchange_name,
                            const std::string& queue_name,
                            const std::string& bindkey)
        {
            _mq_client = std::make_shared<MQClient>(user, password, host);
            _mq_client->declareComponents(exchange_name, queue_name, bindkey);
            _exchange_name = exchange_name;
            _bindkey = bindkey;
        }

        // 构造mysql
        void init_mysql_client(const std::string& user,
                                const std::string& password,
                                const std::string& host,
                                const std::string& db,
                                const std::string& charset,
                                int port,
                                int conn_pool_count)
        {
            _mysql_client = ODBFactory::create(user, password, host, db, charset, port, conn_pool_count);
        }

        // 构造discovery channel
        void init_discovery_client(const std::string& reg_host,
                                    const std::string& base_service_name,
                                    const std::string& user_service_name)
        {
            _user_service_name = user_service_name;
            _channel_manager = std::make_shared<ServiceManager>();
            _channel_manager->declared(_user_service_name);
            
            auto put_cb = [this](const std::string& service_instance, const std::string& host) {
                _channel_manager->onServiceOnline(service_instance, host); 
            };

            auto del_cb = [this](const std::string& service_instance, const std::string& host) {
                _channel_manager->onServiceOffline(service_instance, host); 
            };
            
            _dis_client = std::make_shared<Discovery>(reg_host, base_service_name, put_cb, del_cb);
        }

        // 构造registry
        void init_registry_client(const std::string& reg_host,
                                    const std::string& service_name,
                                    const std::string& access_host)
        {
            _reg_client = std::make_shared<Registry>(reg_host);
            _reg_client->registry(service_name, access_host);
        }

        // 构造 rpc_server
        void init_rpc_server(uint16_t port, int32_t timeout, uint8_t num_threads)
        {
            if (!_mq_client || !_mysql_client || !_channel_manager)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            _rpc_server = std::make_shared<brpc::Server>();

            TransmitServiceImpl* transmit_service = new TransmitServiceImpl(_user_service_name, _channel_manager,
                                                                             _mysql_client, _mq_client, _exchange_name, _bindkey);

            if (-1 == _rpc_server->AddService(transmit_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE))
            {
                LOG_ERROR("添加 rpc 服务失败!");
                abort();
            }

            brpc::ServerOptions options;
            options.idle_timeout_sec = timeout;
            options.num_threads = num_threads;
            if (-1 == _rpc_server->Start(port, &options))
            {
                LOG_ERROR("服务启动失败!");
                abort();
            }
        }

        TransmitServer::ptr build()
        {
            if (!_dis_client || !_reg_client || !_rpc_server || !_mysql_client)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            return std::make_shared<TransmitServer>(_dis_client, _reg_client, _rpc_server, _mysql_client);
        }

    private:
        std::string _user_service_name;
        Discovery::ptr _dis_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
        std::shared_ptr<ServiceManager> _channel_manager;

        std::shared_ptr<odb::core::database> _mysql_client;

        MQClient::ptr _mq_client;
        std::string _exchange_name;
        std::string _bindkey;
    };
}