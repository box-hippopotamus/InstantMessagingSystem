#pragma once

#include <brpc/server.h>
#include <butil/logging.h>
#include <unistd.h>

#include "etcd.hpp"
#include "logger.hpp"
#include "base.pb.h"
#include "file.pb.h"
#include "utils.hpp"


namespace IM
{
    // 文件存储服务RPC接口类
    class FileServiceImpl : public FileService
    {
    public:
        using ptr = std::shared_ptr<FileServiceImpl>;

        FileServiceImpl(const std::string& storage_path)
            : _storage_path(storage_path)
        {
            umask(0);
            mkdir(_storage_path.c_str(), 0775);

            if (_storage_path.back() != '/')
                _storage_path += "/";
        }

        ~FileServiceImpl() = default;

         void GetSingleFile(google::protobuf::RpcController* controller,
                            const ::IM::GetSingleFileReq* request,
                            ::IM::GetSingleFileRsp* response,
                            ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            std::string fid = request->file_id();
            std::string file_name = _storage_path + fid;
            std::string body;

            if (!FileOp::readFile(file_name, body))
            {
                LOG_ERROR("{} 文件读取失败!", fid);
                response->set_success(false);
                response->set_errmsg("文件读取失败!");
                return;
            }

            response->set_success(true);
            response->mutable_file_data()->set_file_id(fid);
            response->mutable_file_data()->set_file_content(body);
        }

        void GetMultiFile(google::protobuf::RpcController* controller,
                          const ::IM::GetMultiFileReq* request,
                          ::IM::GetMultiFileRsp* response,
                          ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            for (int i = 0; i < request->file_id_list_size(); i++)
            {
                std::string fid = request->file_id_list(i);
                std::string file_name = _storage_path + fid;
                std::string body;

                if (!FileOp::readFile(file_name, body))
                {
                    LOG_ERROR("{} 文件读取失败!", fid);
                    response->set_success(false);
                    response->set_errmsg("文件读取失败!");
                    response->clear_file_data();
                    return;
                }

                FileDownloadData data;
                data.set_file_id(fid);
                data.set_file_content(body);
                response->mutable_file_data()->insert({fid, data});
            }

            response->set_success(true);
        }

        void PutSingleFile(google::protobuf::RpcController* controller,
                           const ::IM::PutSingleFileReq* request,
                           ::IM::PutSingleFileRsp* response,
                           ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            std::string fid = UUID::uuid();
            std::string file_name = _storage_path + fid;

            if (!FileOp::writeFile(file_name, request->file_data().file_content()))
            {
                LOG_ERROR("{} 文件写入失败!", fid);
                response->set_success(false);
                response->set_errmsg("文件写入失败!");
                return;
            }

            response->set_success(true);
            response->mutable_file_info()->set_file_id(fid);
            response->mutable_file_info()->set_file_size(request->file_data().file_size());
            response->mutable_file_info()->set_file_name(request->file_data().file_name());
        }

        void PutMultiFile(google::protobuf::RpcController* controller,
                          const ::IM::PutMultiFileReq* request,
                          ::IM::PutMultiFileRsp* response,
                          ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            for (int i = 0; i < request->file_data_size(); i++)
            {
                std::string fid = UUID::uuid();
                std::string file_name = _storage_path + fid;

                if (!FileOp::writeFile(file_name, request->file_data(i).file_content()))
                {
                    LOG_ERROR("{} 文件写入失败!", fid);
                    response->set_success(false);
                    response->set_errmsg("文件写入失败!");
                    response->clear_file_info();
                    return;
                }

                FileMessageInfo* finfo = response->add_file_info();
                finfo->set_file_id(fid);
                finfo->set_file_size(request->file_data(i).file_size());
                finfo->set_file_name(request->file_data(i).file_name());
            }

            response->set_success(true);
        }
    private:
        std::string _storage_path;
    };

    // 文件服务类
    class FileServer
    {
    public:
        using ptr = std::shared_ptr<FileServer>;

        FileServer(const Registry::ptr& reg_client, 
                   const std::shared_ptr<brpc::Server>& rpc_server)
            : _reg_client(reg_client)
            , _rpc_server(rpc_server)
        {}

        ~FileServer() = default;

        // 启动rpc服务
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };

    class FileServiceBuilder
    {
    public:
        // 构造服务注册客户端
        void init_reg_client(const std::string& reg_host,    // 注册中心地址
                      const std::string& servicename,  // 服务名称
                      const std::string& access_host) // 服务提供地址
        {
            _reg_client = std::make_shared<Registry>(reg_host);
            _reg_client->registry(servicename, access_host);
        }

        // 构造rpc服务端
        void init_rpc_server(uint16_t port, uint32_t timeout, uint8_t num_threads, const std::string& storage_path = "./data")
        {
            _rpc_server = std::make_shared<brpc::Server>();

            // 添加服务
            FileServiceImpl *file_service = new FileServiceImpl(storage_path);
            if (_rpc_server->AddService(file_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE) == -1)
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

       FileServer::ptr build()
        {
            if (!_reg_client || !_rpc_server)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            return std::make_shared<FileServer>(_reg_client, _rpc_server);
        }

    private:
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };
}