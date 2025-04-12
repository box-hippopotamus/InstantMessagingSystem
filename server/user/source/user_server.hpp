#pragma once

#include <cctype>

#include <brpc/server.h>
#include <butil/logging.h>

#include "dms.hpp"
#include "utils.hpp"
#include "etcd.hpp"
#include "logger.hpp"
#include "channel.hpp"
#include "es_user.hpp"
#include "mysql_user.hpp"
#include "redis_user.hpp"

#include "user.hxx"
#include "user-odb.hxx"

#include "base.pb.h"
#include "user.pb.h"
#include "file.pb.h"

namespace IM
{
    // 用户服务RPC接口类
    class UserServiceImpl : public UserService
    {
    public:
        using ptr = std::shared_ptr<UserServiceImpl>;

        UserServiceImpl(const std::shared_ptr<elasticlient::Client>& es_client,
                        const std::shared_ptr<odb::core::database>& mysql_client,
                        const std::shared_ptr<sw::redis::Redis>& redis_client,
                        const ServiceManager::ptr& channel_manager,
                        const std::string& file_service_name,
                        const DMSClient::ptr& dms_client)
            : _es_user(std::make_shared<ESUser>(es_client))
            , _mysql_user(std::make_shared<UserTable>(mysql_client))
            , _redis_session(std::make_shared<Session>(redis_client))
            , _redis_status(std::make_shared<Status>(redis_client))
            , _redis_codes(std::make_shared<Codes>(redis_client))
            , _file_service_name(file_service_name)
            , _channel_manager(channel_manager)
            , _dms_client(dms_client)
        {
            _es_user->createIndex();
        }

        ~UserServiceImpl() = default;
        
        void UserRegister(google::protobuf::RpcController* controller,
                                    const ::IM::UserRegisterReq* request,
                                    ::IM::UserRegisterRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string& errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            std::string nickname = request->nickname();
            std::string password = request->password();

            // 检查用户名
            if (!nicknameCheck(nickname)) 
            {
                LOG_ERROR("{}: 用户名非法!", request->request_id());
                return err_response("用户名非法!");
            }

            // 检查密码
            if (!passwordCheck(password))
            {
                LOG_ERROR("{}: 密码非法!", request->request_id());
                return err_response("密码非法!");
            }

            // 检查数据库
            auto user = _mysql_user->select_by_nickname(nickname);
            if (user)
            {
                LOG_ERROR("{}: 用户名 {} 已存在!", request->request_id(), nickname);
               
                return err_response("用户名已存在!");
            }

            // MySQL 插入数据
            std::string uid = UUID::uuid();
            user = std::make_shared<User>(uid, nickname, password);
            if (!_mysql_user->insert(user))
            {
                LOG_ERROR("{}: MySQL 新增失败!", request->request_id());
                return err_response("数据库新增失败!");
            }

            // es 插入数据
            if (!_es_user->appendData(uid, "", nickname, "", ""))
            {
                LOG_ERROR("{}: es 新增失败!", request->request_id());
                return err_response("搜索引擎新增失败!");
            }

            response->set_success(true);
        }

        void UserLogin(google::protobuf::RpcController* controller,
                                const ::IM::UserLoginReq* request,
                                ::IM::UserLoginRsp* response,
                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string& errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            std::string nickname = request->nickname();
            std::string password = request->password();

            // 查询数据库
            auto user = _mysql_user->select_by_nickname(nickname);
            if (!user || password != user->password())
            {
                LOG_ERROR("{}: 用户名或密码错误!", request->request_id());
                return err_response("用户名或密码错误!");
            }

            // 检查是否已经登录
            if (_redis_status->exists(user->userId()))
            {
                LOG_ERROR("{}: {} 用户已在其它地方登录!", request->request_id(), nickname);
                return err_response("用户已在其它地方登录!");
            }

            // 构造会话id
            std::string ssid = UUID::uuid();
            _redis_session->append(ssid, user->userId());
            _redis_status->append(user->userId());

            response->set_login_session_id(ssid);
            response->set_success(true);
        }

        void GetPhoneVerifyCode(google::protobuf::RpcController* controller,
                                        const ::IM::PhoneVerifyCodeReq* request,
                                        ::IM::PhoneVerifyCodeRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string& errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出手机号码
            std::string phone = request->phone_number();

            // 验证手机号码格式是否正确（必须以 1 开始，第二位 3~9 之间，后边 9 个数字字符）
            if (!phoneCheck(phone)) 
            {
                LOG_ERROR("{}: {} 手机号码格式错误!", request->request_id(), phone);
                return err_response("手机号码格式错误!");
            }

            // 生成 4 位随机验证码
            std::string code_id = UUID::uuid();
            std::string code = UUID::vcode();

            // 发送验证码
            if (!_dms_client->send(phone, code)) 
            {
                LOG_ERROR("{}: {} 短信验证码发送失败!", request->request_id(), phone);
                return err_response("短信验证码发送失败!");
            }

            // 构造验证码 ID，添加到 redis 验证码映射键值索引中
            _redis_codes->append(code_id, code);

            response->set_verify_code_id(code_id);
            response->set_success(true);
        }

        void PhoneRegister(google::protobuf::RpcController* controller,
                                    const ::IM::PhoneRegisterReq* request,
                                    ::IM::PhoneRegisterRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response] (const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出手机号码和验证码,验证码ID
            std::string phone = request->phone_number();
            std::string code_id = request->verify_code_id();
            std::string code = request->verify_code();

            // 检查注册手机号码是否合法
            if (!phoneCheck(phone)) 
            {
                LOG_ERROR("{}: {} 手机号码格式错误!", request->request_id(), phone);
                return err_response("手机号码格式错误!");
            }

            // 从 redis 数据库中进行验证码 ID-验证码一致性匹配
            if (_redis_codes->code(code_id) != code) 
            {
                LOG_ERROR("{}: {} - {} 验证码错误!", request->request_id(), code_id, code);
                return err_response("验证码错误!");
            }

            // 通过数据库查询判断手机号是否已经注册过
            if (_mysql_user->select_by_phone(phone)) 
            {
                LOG_ERROR("{}: {} 手机号已注册!", request->request_id(), phone);
                return err_response("该手机号已注册!");
            }

            // 向数据库新增用户信息
            std::string uid = UUID::uuid();
            auto user = std::make_shared<User>(uid, phone);
            if (!_mysql_user->insert(user)) 
            {
                LOG_ERROR("{}: {} 向数据库添加用户失败!", request->request_id(), phone);
                return err_response("数据库新增数据失败!");
            }

            // 向 ES 服务器中新增用户信息
            if (!_es_user->appendData(uid, phone, uid, "", "")) 
            {
                LOG_ERROR("{}: 搜索引擎新增数据失败!", request->request_id());
                return err_response("搜索引擎新增数据失败!");
            }

            response->set_success(true);
        }

        void PhoneLogin(google::protobuf::RpcController* controller,
                                const ::IM::PhoneLoginReq* request,
                                ::IM::PhoneLoginRsp* response,
                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };
            // 从请求中取出手机号码和验证码 ID，以及验证码。
            std::string phone = request->phone_number();
            std::string code_id = request->verify_code_id();
            std::string code = request->verify_code();

            // 检查注册手机号码是否合法
            if (!phoneCheck(phone))
            {
                LOG_ERROR("{}: {}手机号码格式错误!", request->request_id(), phone);
                return err_response("手机号码格式错误!");
            }

            // 判断用用户是否存在
            auto user = _mysql_user->select_by_phone(phone);
            if (!user) 
            {
                LOG_ERROR("{}: {}手机号未注册!", request->request_id(), phone);
                return err_response("手机号未注册!");
            }

            // 从 redis 数据库中检验验证码
            if (_redis_codes->code(code_id) != code) 
            {
                LOG_ERROR("{}: {}验证码错误!", request->request_id(), code);
                return err_response("验证码错误!");
            }

            _redis_codes->remove(code_id);

            // 判断用户是否已经登录
            if (_redis_status->exists(user->userId())) 
            {
                LOG_ERROR("{}: {}用户已在其他位置登录!", request->request_id(), phone);
                return err_response("用户已在其他位置登录!");
            }

            // 构造会话 ID，生成会话键值对，向 redis 中添加会话信息
            std::string ssid = UUID::uuid();
            _redis_session->append(ssid, user->userId());

            // 添加用户登录信息
            _redis_status->append(user->userId());

            // 组织响应，返回生成的会话 ID
            response->set_login_session_id(ssid);
            response->set_success(true);
        }

        void GetUserInfo(google::protobuf::RpcController* controller,
                                    const ::IM::GetUserInfoReq* request,
                                    ::IM::GetUserInfoRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出用户 ID
            std::string uid = request->user_id();

            // 通过用户 ID，从数据库中查询用户信息
            auto user = _mysql_user->select_by_id(uid);
            if (!user) 
            {
                LOG_ERROR("{}: {}未找到用户信息!", request->request_id(), uid);
                return err_response("未找到用户信息!");
            }

            // 根据用户信息中的头像 ID，从文件服务器获取头像文件数据，组织完整用户信息
            UserInfo *user_info = response->mutable_user_info();
            user_info->set_user_id(user->userId());
            user_info->set_nickname(user->nickname());
            user_info->set_description(user->description());
            user_info->set_phone(user->phone());
            
            if (!user->avatarId().empty()) 
            {
                // 从信道管理对象中，获取到连接了文件管理子服务的channel
                auto channel = _channel_manager->choose(_file_service_name);
                if (!channel) 
                {
                    LOG_ERROR("{}: {}未找到文件管理子服务节点!", request->request_id(), _file_service_name);
                    return err_response("未找到文件管理子服务节点!");
                }

                // 进行文件子服务的rpc请求，进行头像文件下载
                FileService_Stub stub(channel.get());
                GetSingleFileReq req;
                GetSingleFileRsp rsp;
                req.set_request_id(request->request_id());
                req.set_file_id(user->avatarId());

                brpc::Controller cntl;
                stub.GetSingleFile(&cntl, &req, &rsp, nullptr);
                if (cntl.Failed() == true || rsp.success() == false) 
                {
                    LOG_ERROR("{}: {}文件子服务调用失败!", request->request_id(), cntl.ErrorText());
                    return err_response("文件子服务调用失败!");
                }
                user_info->set_avatar(rsp.file_data().file_content());
            }

            // 组织响应，返回用户信息
            response->set_success(true);
        }

        void GetMultiUserInfo(google::protobuf::RpcController* controller,
                                        const ::IM::GetMultiUserInfoReq* request,
                                        ::IM::GetMultiUserInfoRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出用户ID
            std::vector<std::string> uid_lists;
            for (int i = 0; i < request->users_id_size(); i++) 
                uid_lists.push_back(request->users_id(i));

            // 从数据库进行批量用户信息查询
            auto users = _mysql_user->select_by_ids(uid_lists);
            if (users.size() != request->users_id_size()) 
            {
                LOG_ERROR("{}: 从数据库查找的用户信息数量不一致!", request->request_id());
                return err_response("数据库查询有误!");
            }

            // 批量从文件管理子服务进行文件下载
            auto channel = _channel_manager->choose(_file_service_name);
            if (!channel) 
            {
                LOG_ERROR("{}: {}文件子服务调用失败!", request->request_id(), _file_service_name);
                return err_response("文件子服务调用失败!");
            }

            FileService_Stub stub(channel.get());
            GetMultiFileReq req;
            GetMultiFileRsp rsp;
            req.set_request_id(request->request_id());

            for (auto &user : users) 
            {
                if (!user.avatarId().empty())
                    req.add_file_id_list(user.avatarId());
            }

            brpc::Controller cntl;
            stub.GetMultiFile(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || !rsp.success()) 
            {
                LOG_ERROR("{}: 文件子服务调用失败: {}", request->request_id(), cntl.ErrorText());
                return err_response("文件子服务调用失败!");
            }

            // 组织响应
            auto file_map = rsp.mutable_file_data(); // 批量文件请求响应中的map 
            auto user_map = response->mutable_users_info(); //响应的用户信息map
            for (auto &user : users) 
            {
                UserInfo user_info;
                user_info.set_user_id(user.userId());
                user_info.set_nickname(user.nickname());
                user_info.set_description(user.description());
                user_info.set_phone(user.phone());
                user_info.set_avatar((*file_map)[user.avatarId()].file_content());
                (*user_map)[user_info.user_id()] = user_info;
            }

            response->set_success(true);
        }

        void SetUserAvatar(google::protobuf::RpcController* controller,
                                        const ::IM::SetUserAvatarReq* request,
                                        ::IM::SetUserAvatarRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string& errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出用户 ID 与头像数据
            std::string uid = request->user_id();

            // 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
            auto user = _mysql_user->select_by_id(uid);
            if (!user) 
            {
                LOG_ERROR("{}: {}用户不存在!", request->request_id(), uid);
                return err_response("用户不存在!");
            }

            // 上传头像文件到文件子服务，
            auto channel = _channel_manager->choose(_file_service_name);
            if (!channel) 
            {
                LOG_ERROR("{}: {}文件子服务调用失败!", request->request_id(), _file_service_name);
                return err_response("文件子服务调用失败!");
            }

            FileService_Stub stub(channel.get());
            PutSingleFileReq req;
            PutSingleFileRsp rsp;
            req.set_request_id(request->request_id());
            req.mutable_file_data()->set_file_name("");
            req.mutable_file_data()->set_file_size(request->avatar().size());
            req.mutable_file_data()->set_file_content(request->avatar());

            brpc::Controller cntl;
            stub.PutSingleFile(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || !rsp.success())
            {
                LOG_ERROR("{}: 文件子服务调用失败: {}", request->request_id(), cntl.ErrorText());
                return err_response("文件子服务调用失败!");
            }

            std::string avatar_id = rsp.file_info().file_id();

            // 将返回的头像文件 ID 更新到数据库中
            user->avatarId(avatar_id);
            if (!_mysql_user->update(user)) 
            {
                LOG_ERROR("{}: {}更新数据库用户头像ID失败!", request->request_id(), avatar_id);
                return err_response("更新数据库失败!");
            }

            // 更新 ES 服务器中用户信息
            if (!_es_user->appendData(user->userId(), user->phone(), user->nickname(), user->description(), user->avatarId())) 
            {
                LOG_ERROR("{}: ES搜索引擎用户头像ID失败: {}", request->request_id(), avatar_id);
                return err_response("更新搜索引擎失败!");
            }

            // 响应
            response->set_success(true);
        }

        void SetUserNickname(google::protobuf::RpcController* controller,
                                        const ::IM::SetUserNicknameReq* request,
                                        ::IM::SetUserNicknameRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string& errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出用户 ID 与新的昵称
            std::string uid = request->user_id();
            std::string new_nickname = request->nickname();

            // 检查昵称格式
            if (!nicknameCheck(new_nickname)) 
            {
                LOG_ERROR("{}: 用户名长度非法!", request->request_id());
                return err_response("用户名长度非法!");
            }

            // 判断用户是否存在
            auto user = _mysql_user->select_by_id(uid);
            if (!user)
            {
                LOG_ERROR("{}: {}用户不存在!", request->request_id(), uid);
                return err_response("用户不存在!");
            }

            // 将新的昵称插入数据库
            user->nickname(new_nickname);
            if (!_mysql_user->update(user)) 
            {
                LOG_ERROR("{}: {}更新数据库用户昵称失败!", request->request_id(), new_nickname);
                return err_response("更新用户昵称失败!");
            }

            // 更新 ES 服务器中用户信息
            if (!_es_user->appendData(user->userId(), user->phone(), user->nickname(), user->description(), user->avatarId())) 
            {
                LOG_ERROR("{}: ES搜索引擎用户头像ID失败: {}", request->request_id(), new_nickname);
                return err_response("更新搜索引擎失败!");
            }

            // 组织响应
            response->set_success(true);
        }

        void SetUserDescription(google::protobuf::RpcController* controller,
                                        const ::IM::SetUserDescriptionReq* request,
                                        ::IM::SetUserDescriptionRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());
            
            auto err_response = [this, response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 取出用户 ID 与新的昵称
            std::string uid = request->user_id();
            std::string new_description = request->description();

            // 判断用户是否存在
            auto user = _mysql_user->select_by_id(uid);
            if (!user) 
            {
                LOG_ERROR("{}: 用户不存在!", request->request_id(), uid);
                return err_response("用户不存在!");
            }

            //新的昵称插入数据库
            user->description(new_description);
            if (!_mysql_user->update(user)) 
            {
                LOG_ERROR("{}: {}数据库插入用户签名失败!", request->request_id(), new_description);
                return err_response("更新用户签名失败!");
            }

            // 更新 ES 服务器中用户信息
            if (!_es_user->appendData(user->userId(), user->phone(), user->nickname(), user->description(), user->avatarId()))
            {
                LOG_ERROR("{}: {}更新搜索引擎用户签名失败!", request->request_id(), new_description);
                return err_response("更新用户签名失败!");
            }

            // 响应
            response->set_success(true);
        }

        void SetUserPhoneNumber(google::protobuf::RpcController* controller,
                                        const ::IM::SetUserPhoneNumberReq* request,
                                        ::IM::SetUserPhoneNumberRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [this, response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 从请求中取出用户 ID 与新的昵称
            std::string uid = request->user_id();
            std::string new_phone = request->phone_number();
            std::string code = request->phone_verify_code();
            std::string code_id = request->phone_verify_code_id();

            // 对验证码进行验证
            if (_redis_codes->code(code_id) != code) 
            {
                LOG_ERROR("{}: {}验证码错误!", request->request_id(), code);
                return err_response("验证码错误!");
            }

            // 判断用户是否存在
            auto user = _mysql_user->select_by_id(uid);
            if (!user) 
            {
                LOG_ERROR("{}: {}用户不存在!", request->request_id(), uid);
                return err_response("用户不存在!");
            }

            // 更新到数据库
            user->phone(new_phone);
            if (!_mysql_user->update(user)) 
            {
                LOG_ERROR("{}: {}更新数据库用户手机号失败!", request->request_id(), new_phone);
                return err_response("更新手机号失败!");
            }

            // 更新 ES 服务器中用户信息
            if (!_es_user->appendData(user->userId(), user->phone(), user->nickname(), user->description(), user->avatarId())) 
            {
                LOG_ERROR("{}: {}更新搜索引擎用户手机号失败!", request->request_id(), new_phone);
                return err_response("更新手机号失败!");
            }

            // 组织响应
            response->set_success(true);
        }

    private:
        bool nicknameCheck(const std::string& nickname)
        {
            return nickname.size() <= 21;
        }

        bool passwordCheck(const std::string& password)
        {
            size_t sz = password.size();

            if (sz > 15 || sz < 6)
            {
                LOG_ERROR("密码长度非法!");
                return false;
            }

            for (auto ch : password)
            {
                if (!isalnum(ch) && ch != '_')
                {
                    LOG_ERROR("{} 字符非法!", ch);
                    return false;
                }
            }

            return true;
        }

        bool phoneCheck(const std::string& phone)
        {
            if (phone.size() != 11 || phone[0] != '1' 
                || phone[1] < '3' || phone[1] > '9')
                return false;

            for (int i = 2; i < 11; i++)
            {
                if (!isdigit(phone[i]))
                    return false;
            }

            return true;
        }

    private:
        // 数据库服务
        ESUser::ptr _es_user;
        UserTable::ptr _mysql_user;
        Session::ptr _redis_session;
        Status::ptr _redis_status;
        Codes::ptr _redis_codes;

        // rpc调用客户端
        std::string _file_service_name;
        std::shared_ptr<ServiceManager> _channel_manager;

        // 短信验证
        DMSClient::ptr _dms_client;
    };

    // 用户服务类
    class UserServer
    {
    public:
        using ptr = std::shared_ptr<UserServer>;

        UserServer(const Discovery::ptr& dis_client, 
                     const Registry::ptr& reg_client, 
                     const std::shared_ptr<brpc::Server>& rpc_server,
                     const std::shared_ptr<elasticlient::Client> es_client,
                     const std::shared_ptr<odb::core::database> mysql_client,
                     const std::shared_ptr<sw::redis::Redis> redis_client)
            : _dis_client(dis_client)
            , _reg_client(reg_client)
            , _rpc_server(rpc_server)
            , _mysql_client(mysql_client)
            , _es_client(es_client)
            , _redis_client(redis_client)
        {}

        ~UserServer() = default;

        // 启动rpc服务
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<odb::core::database> _mysql_client;
        std::shared_ptr<sw::redis::Redis> _redis_client;
        
        Discovery::ptr _dis_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;

        DMSClient::ptr _dms_client;
    };

    class UserServiceBuilder
    {
    public:
        // 构造dms
        void init_dms_client(const std::string& access_key_id, const std::string& access_key_secret, const std::string& ca_path)
        {
            _dms_client = std::make_shared<DMSClient>(access_key_id, access_key_secret, ca_path);
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

        // 构造redis
        void init_redis_client(const std::string& host, 
                                int port, 
                                int db, 
                                bool keep_alive)
        {
            _redis_client = RedisClientFactory::create(host, port, db, keep_alive);
        }

        // 构造discovery channel
        void init_discovery_client(const std::string& reg_host,
                                    const std::string& base_service_name,
                                    const std::string& file_service_name)
        {
            _file_service_name = file_service_name;
            _channel_manager = std::make_shared<ServiceManager>();
            _channel_manager->declared(file_service_name);
            
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
            if (!_es_client || !_mysql_client || !_redis_client 
                || !_dms_client || !_channel_manager)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            _rpc_server = std::make_shared<brpc::Server>();

            UserServiceImpl* user_service = new UserServiceImpl(_es_client, _mysql_client, _redis_client, 
                                                    _channel_manager, _file_service_name, _dms_client);

            if (-1 == _rpc_server->AddService(user_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE))
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

        UserServer::ptr build()
        {
            if (!_dis_client || !_reg_client || !_rpc_server)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            return std::make_shared<UserServer>(_dis_client, _reg_client, _rpc_server,
                                                _es_client, _mysql_client, _redis_client);
        }

    private:
        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<odb::core::database> _mysql_client;
        std::shared_ptr<sw::redis::Redis> _redis_client;

        std::shared_ptr<ServiceManager> _channel_manager;
        
        std::string _file_service_name;
        Discovery::ptr _dis_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;

        DMSClient::ptr _dms_client;
    };
}