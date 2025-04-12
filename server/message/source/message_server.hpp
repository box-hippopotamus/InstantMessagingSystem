#pragma once

#include <cctype>

#include <brpc/server.h>
#include <butil/logging.h>

#include "utils.hpp"
#include "etcd.hpp"
#include "logger.hpp"
#include "channel.hpp"
#include "es_message.hpp"
#include "mysql_message.hpp"
#include "rabbitmq.hpp"

#include "base.pb.h"
#include "message.pb.h"
#include "file.pb.h"
#include "user.pb.h"

namespace IM
{
    // 用户服务RPC接口类
    class MessageServiceImpl : public MsgStorageService
    {
    public:
        using ptr = std::shared_ptr<MessageServiceImpl>;

        MessageServiceImpl(const std::shared_ptr<elasticlient::Client>& es_client,
                        const std::shared_ptr<odb::core::database>& mysql_client,
                        const ServiceManager::ptr& channel_manager,
                        const std::string& file_service_name,
                        const std::string& user_service_name)
            : _es_message(std::make_shared<ESMessage>(es_client))
            , _mysql_message(std::make_shared<MessageTable>(mysql_client))
            , _channel_manager(channel_manager)
            , _file_service_name(file_service_name)
            , _user_service_name(user_service_name)
        {
            _es_message->createIndex();
        }

        ~MessageServiceImpl() = default;
 
        virtual void GetHistoryMsg(google::protobuf::RpcController* controller,
                                    const ::IM::GetHistoryMsgReq* request,
                                    ::IM::GetHistoryMsgRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取会话ID，起始时间，结束时间
            std::string session_id = request->chat_session_id();
            boost::posix_time::ptime stime = boost::posix_time::from_time_t(request->start_time());
            boost::posix_time::ptime etime = boost::posix_time::from_time_t(request->over_time());

            // 查询数据库
            auto msg_lists = _mysql_message->range(session_id, stime, etime);
            if (msg_lists.empty()) 
            {
                response->set_success(true);
                return;
            }

            // 文件服务批量下载文件
            std::unordered_set<std::string> file_id_lists;
            for (const auto& msg : msg_lists) 
            {
                if (msg.fileId().empty()) 
                    continue;
                LOG_DEBUG("需要下载的文件ID: {}", msg.fileId());
                file_id_lists.insert(msg.fileId());
            }

            std::unordered_map<std::string, std::string> file_data_lists;
            if (!getFile(file_id_lists, file_data_lists)) 
            {
                LOG_ERROR("{} 批量文件数据下载失败!", session_id);
                return err_response("批量文件数据下载失败!");
            }

            // 用户服务批量获取用户信息 -> rsp中指明消息接收方
            std::unordered_set<std::string> user_id_lists;
            for (const auto& msg : msg_lists) 
                user_id_lists.insert(msg.userId());

            std::unordered_map<std::string, UserInfo> user_lists;
            if (!getUser(user_id_lists, user_lists)) 
            {
                LOG_ERROR("{} 批量用户数据获取失败!", session_id);
                return err_response("批量用户数据获取失败!");
            }

            // 响应
            response->set_success(true);
            for (const auto& msg : msg_lists) 
            {
                auto message_info = response->add_msg_list();
                message_info->set_message_id(msg.messageId());
                message_info->set_chat_session_id(msg.sessionId());
                message_info->set_timestamp(boost::posix_time::to_time_t(msg.createTime()));
                message_info->mutable_sender()->CopyFrom(user_lists[msg.userId()]);
                message_info->mutable_message()->set_message_type((IM::MessageType)msg.messageType());

                switch(msg.messageType()) 
                {
                    case MessageType::STRING:
                        message_info->mutable_message()->mutable_string_message()->set_content(msg.content());
                        break;
                    case MessageType::IMAGE:
                        message_info->mutable_message()->mutable_image_message()->set_file_id(msg.fileId());
                        message_info->mutable_message()->mutable_image_message()->set_image_content(file_data_lists[msg.fileId()]);
                        break;
                    case MessageType::FILE:
                        message_info->mutable_message()->mutable_file_message()->set_file_id(msg.fileId());
                        message_info->mutable_message()->mutable_file_message()->set_file_size(msg.fileSize());
                        message_info->mutable_message()->mutable_file_message()->set_file_name(msg.fileName());
                        message_info->mutable_message()->mutable_file_message()->set_file_contents(file_data_lists[msg.fileId()]);
                        break;
                    case MessageType::SPEECH:
                        message_info->mutable_message()->mutable_speech_message()->set_file_id(msg.fileId());
                        message_info->mutable_message()->mutable_speech_message()->set_file_contents(file_data_lists[msg.fileId()]);
                        break;
                    default:
                        LOG_ERROR("消息类型错误!");
                        return;
                }
            }
        }

        virtual void GetRecentMsg(google::protobuf::RpcController* controller,
                                    const ::IM::GetRecentMsgReq* request,
                                    ::IM::GetRecentMsgRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取会话ID，消息数量
            std::string session_id = request->chat_session_id();
            int message_count = request->msg_count();

            // 查询数据库
            auto msg_lists = _mysql_message->recent(session_id, message_count);
            if (msg_lists.empty()) 
            {
                response->set_success(true);
                return;
            }

            // 文件服务批量下载文件
            std::unordered_set<std::string> file_id_lists;
            for (const auto& msg : msg_lists) 
            {
                if (msg.fileId().empty()) 
                    continue;
                LOG_DEBUG("需要下载的文件ID: {}", msg.fileId());
                file_id_lists.insert(msg.fileId());
            }

            std::unordered_map<std::string, std::string> file_data_lists;
            if (!getFile(file_id_lists, file_data_lists)) 
            {
                LOG_ERROR("{} 批量文件数据下载失败!", session_id);
                return err_response("批量文件数据下载失败!");
            }

            // 用户服务批量获取用户信息 -> rsp中指明消息接收方
            std::unordered_set<std::string> user_id_lists;
            for (const auto& msg : msg_lists) 
                user_id_lists.insert(msg.userId());

            std::unordered_map<std::string, UserInfo> user_lists;
            if (!getUser(user_id_lists, user_lists)) 
            {
                LOG_ERROR("{} 批量用户数据获取失败!", session_id);
                return err_response("批量用户数据获取失败!");
            }

            // 响应
            response->set_success(true);
            for (const auto& msg : msg_lists) 
            {
                auto message_info = response->add_msg_list();
                message_info->set_message_id(msg.messageId());
                message_info->set_chat_session_id(msg.sessionId());
                message_info->set_timestamp(boost::posix_time::to_time_t(msg.createTime()));
                message_info->mutable_sender()->CopyFrom(user_lists[msg.userId()]);
                message_info->mutable_message()->set_message_type((IM::MessageType)msg.messageType());

                switch(msg.messageType()) 
                {
                    case MessageType::STRING:
                        message_info->mutable_message()->mutable_string_message()->set_content(msg.content());
                        break;
                    case MessageType::IMAGE:
                        message_info->mutable_message()->mutable_image_message()->set_file_id(msg.fileId());
                        message_info->mutable_message()->mutable_image_message()->set_image_content(file_data_lists[msg.fileId()]);
                        break;
                    case MessageType::FILE:
                        message_info->mutable_message()->mutable_file_message()->set_file_id(msg.fileId());
                        message_info->mutable_message()->mutable_file_message()->set_file_size(msg.fileSize());
                        message_info->mutable_message()->mutable_file_message()->set_file_name(msg.fileName());
                        message_info->mutable_message()->mutable_file_message()->set_file_contents(file_data_lists[msg.fileId()]);
                        break;
                    case MessageType::SPEECH:
                        message_info->mutable_message()->mutable_speech_message()->set_file_id(msg.fileId());
                        message_info->mutable_message()->mutable_speech_message()->set_file_contents(file_data_lists[msg.fileId()]);
                        break;
                    default:
                        LOG_ERROR("消息类型错误!");
                        return;
                }
            }
        }
        
        virtual void MsgSearch(google::protobuf::RpcController* controller,
                                const ::IM::MsgSearchReq* request,
                                ::IM::MsgSearchRsp* response,
                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 关键字的消息搜索 -> 结果只有文本消息
            // 提取会话ID, 关键字
            std::string session_id = request->chat_session_id();
            std::string skey = request->search_key();

            // ES关键字消息搜索
            auto msg_lists = _es_message->search(skey, session_id);
            if (msg_lists.empty()) 
            {
                response->set_success(true);
                return;
            }

            // 获取用户信息
            std::unordered_set<std::string> user_id_lists;
            for (const auto& msg : msg_lists)
                user_id_lists.insert(msg.userId());

            std::unordered_map<std::string, UserInfo> user_lists;
            if (!getUser(user_id_lists, user_lists)) 
            {
                LOG_ERROR("{} 批量用户数据获取失败!", session_id);
                return err_response("批量用户数据获取失败!");
            }

            // 响应 
            response->set_success(true);
            for (const auto& msg : msg_lists) 
            {
                auto message_info = response->add_msg_list();
                message_info->set_message_id(msg.messageId());
                message_info->set_chat_session_id(msg.sessionId());
                message_info->set_timestamp(boost::posix_time::to_time_t(msg.createTime()));
                message_info->mutable_sender()->CopyFrom(user_lists[msg.userId()]);
                message_info->mutable_message()->set_message_type(MessageType::STRING);
                message_info->mutable_message()->mutable_string_message()->set_content(msg.content());
            }
        }

        void onMessage(const char* body, size_t sz)
        {
            // LOG_INFO("收到MQ消息!");
            
            // 反序列化
            IM::MessageInfo message;
            if (!message.ParseFromArray(body, sz)) 
            {
                LOG_ERROR("消息反序列化失败!");
                return;
            }

            // 根据消息类型进行处理
            int64_t file_size;
            std::string file_id, file_name, content;
            auto type = message.message().message_type();
            if (type == MessageType::STRING)
            {
                content = message.message().string_message().content();
                bool ret = _es_message->appendData(
                            message.sender().user_id(),
                            message.message_id(),
                            message.chat_session_id(),
                            message.timestamp(),
                            content);
                if (ret == false) 
                {
                    LOG_ERROR("文本消息存储失败!");
                    return;
                }
            }
            else if (MessageType::IMAGE)
            {
                const auto& msg = message.message().image_message();
                if (!putFile("", msg.image_content(), msg.image_content().size(), file_id)) 
                {
                    LOG_ERROR("上传图片到文件子服务失败!");
                    return;
                }
            }
            else if (MessageType::FILE)
            {
                const auto& msg = message.message().file_message();
                file_name = msg.file_name();
                file_size = msg.file_size();
                if (!putFile(file_name, msg.file_contents(), file_size, file_id))
                {
                    LOG_ERROR("上传文件到文件子服务失败!");
                    return ;
                }
            }
            else if (MessageType::SPEECH)
            {
                const auto &msg = message.message().speech_message();
                if (!putFile("", msg.file_contents(), msg.file_contents().size(), file_id)) 
                {
                    LOG_ERROR("上传语音到文件子服务失败!");
                    return ;
                }
            }
            else
            {
                LOG_ERROR("消息类型错误!");
                return;
            }

            // 存储mysql数据库
            IM::Message msg(message.message_id(), message.chat_session_id(), message.sender().user_id(),
                message.message().message_type(), boost::posix_time::from_time_t(message.timestamp()));

            msg.content(content);
            msg.fileId(file_id);
            msg.fileName(file_name);
            msg.fileSize(file_size);
            if (!_mysql_message->insert(msg)) 
            {
                LOG_ERROR("向数据库插入新消息失败!");
                return;
            }
        }

    private:
        bool getUser(const std::unordered_set<std::string> &user_id_lists,
                        std::unordered_map<std::string, UserInfo> &user_lists) 
        {
            auto channel = _channel_manager->choose(_user_service_name);
            if (!channel) 
            {
                LOG_ERROR("{} 没有可供访问的用户子服务节点!",  _user_service_name);
                return false;
            }

            UserService_Stub stub(channel.get());
            GetMultiUserInfoReq req;
            GetMultiUserInfoRsp rsp;
            req.set_request_id(UUID::uuid());
            for (const auto &id : user_id_lists) 
                req.add_users_id(id);

            brpc::Controller cntl;
            stub.GetMultiUserInfo(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || !rsp.success()) 
            {
                LOG_ERROR("用户子服务调用失败: {}", cntl.ErrorText());
                return false;
            }

            const auto& umap = rsp.users_info();
            for (auto it = umap.begin(); it != umap.end(); ++it) 
                user_lists.insert({it->first, it->second});

            return true;
        }

        bool getFile(const std::unordered_set<std::string> &file_id_lists,
                        std::unordered_map<std::string, std::string> &file_data_lists) 
        {
            auto channel = _channel_manager->choose(_file_service_name);
            if (!channel) 
            {
                LOG_ERROR("{} 没有可供访问的文件子服务节点!",  _file_service_name);
                return false;
            }

            FileService_Stub stub(channel.get());
            GetMultiFileReq req;
            GetMultiFileRsp rsp;
            req.set_request_id(UUID::uuid());
            for (const auto& id : file_id_lists)
                req.add_file_id_list(id);

            brpc::Controller cntl;
            stub.GetMultiFile(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || !rsp.success()) 
            {
                LOG_ERROR("文件子服务调用失败: {}", cntl.ErrorText());
                return false;
            }

            const auto& fmap = rsp.file_data();
            for (auto it = fmap.begin(); it != fmap.end(); ++it) 
                file_data_lists.insert({it->first, it->second.file_content()});

            return true;
        }

        // 文件数据上传
        bool putFile(const std::string &filename, const std::string &body, 
                        const int64_t fsize, std::string &file_id) 
        {
            auto channel = _channel_manager->choose(_file_service_name);
            if (!channel) 
            {
                LOG_ERROR("{} 没有可供访问的文件子服务节点!",  _file_service_name);
                return false;
            }

            FileService_Stub stub(channel.get());
            PutSingleFileReq req;
            PutSingleFileRsp rsp;
            brpc::Controller cntl;

            req.mutable_file_data()->set_file_name(filename);
            req.mutable_file_data()->set_file_size(fsize);
            req.mutable_file_data()->set_file_content(body);

            stub.PutSingleFile(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || !rsp.success()) 
            {
                LOG_ERROR("文件服务调用失败: {}", cntl.ErrorText());
                return false;
            }

            file_id = rsp.file_info().file_id();
            return true;
        }

    private:
        // 数据库服务
        ESMessage::ptr _es_message;
        MessageTable::ptr _mysql_message;

        // rpc调用客户端
        std::string _file_service_name;
        std::string _user_service_name;
        std::shared_ptr<ServiceManager> _channel_manager;
    };

    // 用户服务类
    class MessageServer
    {
    public:
        using ptr = std::shared_ptr<MessageServer>;

        MessageServer(const MQClient::ptr& mq_client,
                        const Discovery::ptr& dis_client, 
                        const Registry::ptr& reg_client, 
                        const std::shared_ptr<brpc::Server>& rpc_server,
                        const std::shared_ptr<elasticlient::Client> es_client,
                        const std::shared_ptr<odb::core::database> mysql_client)
            : _mq_client(mq_client)
            , _dis_client(dis_client)
            , _reg_client(reg_client)
            , _rpc_server(rpc_server)
            , _mysql_client(mysql_client)
        {}

        ~MessageServer() = default;

        // 启动rpc服务
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<odb::core::database> _mysql_client;
        
        Discovery::ptr _dis_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
        MQClient::ptr _mq_client;
    };

    class MessageServiceBuilder
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
            _queue_name = queue_name;
        }

        // 构造es
        void init_es_client(const std::vector<std::string> host_list)
        {
            _es_client = ESClientFactory::create(host_list);
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
                                    const std::string& file_service_name,
                                    const std::string& user_service_name)
        {
            _file_service_name = file_service_name;
            _user_service_name = user_service_name;
            _channel_manager = std::make_shared<ServiceManager>();
            _channel_manager->declared(_file_service_name);
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
            if (!_mq_client || !_es_client || !_mysql_client || !_channel_manager)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            _rpc_server = std::make_shared<brpc::Server>();

            MessageServiceImpl* message_service = new MessageServiceImpl(_es_client, _mysql_client,
                                                    _channel_manager, _file_service_name, _user_service_name);

            if (-1 == _rpc_server->AddService(message_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE))
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

            _mq_client->consume(_queue_name, [message_service](const char* body, size_t sz){
                message_service->onMessage(body, sz);
            });
        }

        MessageServer::ptr build()
        {
            if (!_dis_client || !_reg_client || !_rpc_server)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            return std::make_shared<MessageServer>(_mq_client, _dis_client, _reg_client, 
                                                    _rpc_server, _es_client, _mysql_client);
        }

    private:
        MQClient::ptr _mq_client;
        std::string _exchange_name;
        std::string _queue_name;

        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<odb::core::database> _mysql_client;

        std::shared_ptr<ServiceManager> _channel_manager;
        
        std::string _file_service_name;
        std::string _user_service_name;
        Discovery::ptr _dis_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };
}