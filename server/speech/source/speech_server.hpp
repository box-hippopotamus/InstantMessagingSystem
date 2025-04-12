#pragma once

#include <brpc/server.h>
#include <butil/logging.h>

#include "asr.hpp"
#include "etcd.hpp"
#include "logger.hpp"
#include "speech.pb.h"

namespace IM
{
    // 语音服务RPC接口类
    class SpeechServiceImpl : public SpeechService
    {
    public:
        using ptr = std::shared_ptr<SpeechServiceImpl>;

        SpeechServiceImpl(const AsrClient::ptr& asr_client)
            : _asr_client(asr_client)
        {}

        ~SpeechServiceImpl() = default;

        void SpeechRecognition(google::protobuf::RpcController* controller,
                                const ::IM::SpeechRecognitionReq* request,
                                ::IM::SpeechRecognitionRsp* response,
                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);

            std::string error_msg;
            std::string res = _asr_client->recognize(request->speech_content(), error_msg);
            response->set_request_id(request->request_id());

            if (res.empty())
            {
                LOG_ERROR("语音识别失败: {}", error_msg);
                response->set_success(false);
                response->set_errmsg("语音识别失败: " + error_msg);
                return;
            }

            response->set_success(true);
            response->set_recognition_result(res);
        }

    private:
        AsrClient::ptr _asr_client;
    };

    // 语音服务类
    class SpeechServer
    {
    public:
        using ptr = std::shared_ptr<SpeechServer>;

        SpeechServer(const AsrClient::ptr& asr_client, 
                     const Registry::ptr& reg_client, 
                     const std::shared_ptr<brpc::Server>& rpc_server)
            : _asr_client(asr_client)
            , _reg_client(reg_client)
            , _rpc_server(rpc_server)
        {}

        ~SpeechServer() = default;

        // 启动rpc服务
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        AsrClient::ptr _asr_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };

    class SpeechServiceBuilder
    {
    public:
        // 构造语音识别客户端
        void init_asr_client(const std::string& app_id,
                      const std::string& api_key,
                      const std::string& access_key)
        {
            _asr_client = std::make_shared<AsrClient>(app_id, api_key, access_key);
        }

        // 构造服务注册客户端
        void init_reg_client(const std::string& reg_host,    // 注册中心地址
                      const std::string& servicename,  // 服务名称
                      const std::string& access_host) // 服务提供地址
        {
            _reg_client = std::make_shared<Registry>(reg_host);
            _reg_client->registry(servicename, access_host);
        }

        // 构造rpc服务端
        void init_rpc_server(uint16_t port, uint32_t timeout, uint8_t num_threads)
        {
            if (!_asr_client)
            {
                LOG_ERROR("asr必须在rpc_server前初始化!");
                abort();
            }

            _rpc_server = std::make_shared<brpc::Server>();

            // 添加服务
            SpeechServiceImpl *speech_service = new SpeechServiceImpl(_asr_client);
            if (_rpc_server->AddService(speech_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE) == -1)
            {
                LOG_ERROR("添加服务失败!");
                abort();
            }

            // 配置服务
            brpc::ServerOptions options;
            options.idle_timeout_sec = timeout;
            options.num_threads = num_threads;

            if (_rpc_server->Start(port, &options) == -1)
            {
                LOG_ERROR("服务启动失败!");
                abort();
            }
        }

        SpeechServer::ptr build()
        {
            if (!_asr_client || !_reg_client || !_rpc_server)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            return std::make_shared<SpeechServer>(_asr_client, _reg_client, _rpc_server);
        }

    private:
        AsrClient::ptr _asr_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };
}