#pragma once

#include <brpc/server.h>
#include <butil/logging.h>

#include "es_user.hpp"
#include "mysql_session_member.hpp"
#include "mysql_chat_session.hpp"
#include "mysql_relation.hpp"
#include "mysql_friend_apply.hpp"
#include "etcd.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "channel.hpp"

#include "friend.pb.h"
#include "base.pb.h"
#include "user.pb.h"
#include "message.pb.h"

namespace IM
{
    // 好友服务RPC接口类
    class FriendServiceImpl : public FriendService 
    {
    public:
        FriendServiceImpl(const std::shared_ptr<elasticlient::Client>& es_client,
                            const std::shared_ptr<odb::core::database>& mysql_client,
                            const ServiceManager::ptr& channel_manager,
                            const std::string& user_service_name,
                            const std::string& message_service_name) 
            : _es_user(std::make_shared<ESUser>(es_client))
            , _mysql_apply(std::make_shared<FriendApplyTable>(mysql_client))
            , _mysql_chat_session(std::make_shared<ChatSessionTable>(mysql_client))
            , _mysql_session_member(std::make_shared<SessionMemberTable>(mysql_client))
            , _mysql_relation(std::make_shared<RelationTable>(mysql_client))
            , _user_service_name(user_service_name)
            , _message_service_name(message_service_name)
            , _channel_manager(channel_manager)
        {}

        ~FriendServiceImpl() = default;
 
        // 获取好友列表
        virtual void GetFriendList(::google::protobuf::RpcController* controller,
                                    const ::IM::GetFriendListReq* request,
                                    ::IM::GetFriendListRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };
            
            // 提取用户ID
            std::string rid = request->request_id();
            std::string uid = request->user_id();

            // 从数据库中查询获取用户的好友ID
            auto friend_id_lists = _mysql_relation->friends(uid);
            std::unordered_set<std::string> user_id_lists;
            for (auto& id : friend_id_lists)
                user_id_lists.insert(id);

            // 从用户子服务批量获取用户信息
            std::unordered_map<std::string, UserInfo> user_list;
            if (!GetUserInfo(rid, user_id_lists, user_list)) 
            {
                LOG_ERROR("{}: 批量获取用户信息失败!", rid);
                return err_response("批量获取用户信息失败!");
            }

            // 响应
            response->set_success(true);
            for (const auto& it : user_list) 
            {
                auto user_info = response->add_friend_list();
                user_info->CopyFrom(it.second);
            }
        }

        // 删除好友
        virtual void FriendRemove(::google::protobuf::RpcController* controller,
                                    const ::IM::FriendRemoveReq* request,
                                    ::IM::FriendRemoveRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };
    
            // 提取用户ID，要删除的好友ID
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string pid = request->peer_id();

            // 删除好友关系
            if (!_mysql_relation->remove(uid, pid)) 
            {
                LOG_ERROR("{}: 从数据库删除好友信息失败!", rid);
                return err_response("从数据库删除好友信息失败!");
            }

            // 删除对应的聊天会话
            if (!_mysql_chat_session->remove(uid, pid)) 
            {
                LOG_ERROR("{}: 从数据库删除好友会话信息失败!", rid);
                return err_response("从数据库删除好友会话信息失败!");
            }

            // 响应
            response->set_success(true);
        }

        // 添加好友
        virtual void FriendAdd(::google::protobuf::RpcController* controller,
                                const ::IM::FriendAddReq* request,
                                ::IM::FriendAddRsp* response,
                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取双方用户ID
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string pid = request->respondent_id();

            // 判断两人是否已经是好友
            if (_mysql_relation->exists(uid, pid)) 
            {
                LOG_ERROR("{}: 申请好友失败,已经是好友关系!", rid);
                return err_response("已经是好友关系!");
            }

            // 是否已经申请过好友
            if (_mysql_apply->exists(uid, pid)) 
            {
                LOG_ERROR("{}: 申请好友失败, {} : {} 已经申请过对方好友!", rid, uid, pid);
                return err_response("已经申请过对方好友!");
            }

            // 新增申请信息
            std::string eid = UUID::uuid();
            FriendApply event(eid, uid, pid);
            if (!_mysql_apply->insert(event)) 
            {
                LOG_ERROR("{}: 向数据库新增好友申请事件失败!", rid);
                return err_response("向数据库新增好友申请事件失败!");
            }

            // 响应
            response->set_success(true);
            response->set_notify_event_id(eid);
        }

        // 处理好友申请
        virtual void FriendAddProcess(::google::protobuf::RpcController* controller,
                                        const ::IM::FriendAddProcessReq* request,
                                        ::IM::FriendAddProcessRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取申请人用户ID；被申请人用户ID,处理结果,事件ID
            std::string rid = request->request_id();
            std::string eid = request->notify_event_id();
            std::string uid = request->user_id();
            std::string pid = request->apply_user_id();
            bool agree = request->agree();

            // 判断有没有该申请事件
            if (!_mysql_apply->exists(pid, uid)) 
            {
                LOG_ERROR("{}: 没有找到 {} : {} 对应的好友申请事件!", rid, pid, uid);
                return err_response("没有找到对应的好友申请事件!");
            }

            // 删除已经处理完毕的事件
            if (!_mysql_apply->remove(pid, uid)) 
            {
                LOG_ERROR("{}: 从数据库删除申请事件 {} : {} 失败!", rid, pid, uid);
                return err_response("从数据库删除申请事件失败!");
            }

            // 同意好友请求
            std::string ssid;
            if (agree) 
            {
                // 添加好友关系
                if (!_mysql_relation->insert(uid, pid)) 
                {
                    LOG_ERROR("{}: 新增好友关系信息 {} : {} 失败!", rid, uid, pid);
                    return err_response("新增好友关系信息失败!");
                }

                ssid = UUID::uuid();
                ChatSession cs(ssid, "", ChatSessionType::SINGLE);
                // 添加会话
                if (!_mysql_chat_session->insert(cs)) 
                {
                    LOG_ERROR("{}: 新增单聊会话信息 {} 失败!", rid, ssid);
                    return err_response("新增单聊会话信息失败!");
                }

                SessionMember csm1(ssid, uid);
                SessionMember csm2(ssid, pid);
                std::vector<SessionMember> mlist = {csm1, csm2};
                // 添加会话成员
                if (!_mysql_session_member->append(mlist)) 
                {
                    LOG_ERROR("{}- 没有找到{}-{}对应的好友申请事件!", rid, pid, uid);
                    return err_response("没有找到对应的好友申请事件!");
                }
            }

            //5. 组织响应
            response->set_success(true);
            response->set_new_session_id(ssid);
        }

        // 好友搜索
        virtual void FriendSearch(::google::protobuf::RpcController* controller,
                                    const ::IM::FriendSearchReq* request,
                                    ::IM::FriendSearchRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取搜索关键字
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string skey = request->search_key();
            
            // 获取用户的好友ID列表
            auto friend_id_lists = _mysql_relation->friends(uid);
            friend_id_lists.push_back(uid); // 过滤掉自己

            // 用户信息搜索,过滤掉当前的好友
            std::unordered_set<std::string> user_id_lists;
            auto search_res = _es_user->search(skey, friend_id_lists);
            for (auto& it : search_res) 
                user_id_lists.insert(it.userId());

            // 批量用户信息获取
            std::unordered_map<std::string, UserInfo> user_list;
            if (!GetUserInfo(rid, user_id_lists, user_list)) 
            {
                LOG_ERROR("{}: 批量获取用户信息失败!", rid);
                return err_response("批量获取用户信息失败!");
            }

            // 响应
            response->set_success(true);
            for (const auto& it : user_list) 
            {
                auto user_info = response->add_user_info();
                user_info->CopyFrom(it.second);
            }
        }

        // 获取好友申请列表
        virtual void GetPendingFriendEventList(::google::protobuf::RpcController* controller,
                                                const ::IM::GetPendingFriendEventListReq* request,
                                                ::IM::GetPendingFriendEventListRsp* response,
                                                ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取当前用户ID
            std::string rid = request->request_id();
            std::string uid = request->user_id();

            // 从数据库获取申请用户ID列表
            auto res = _mysql_apply->apply_users(uid);
            std::unordered_set<std::string> user_id_lists;
            for (auto& id : res) 
                user_id_lists.insert(id);

            // 批量获取申请人用户信息
            std::unordered_map<std::string, UserInfo> user_list;
            if (!GetUserInfo(rid, user_id_lists, user_list)) 
            {
                LOG_ERROR("{}: 批量获取用户信息失败!", rid);
                return err_response("批量获取用户信息失败!");
            }

            // 响应
            response->set_success(true);
            for (const auto & user_it : user_list) 
            {
                auto ev = response->add_event();
                ev->mutable_sender()->CopyFrom(user_it.second);
            }
        }

        // 获取会话列表
        virtual void GetChatSessionList(::google::protobuf::RpcController* controller,
                                        const ::IM::GetChatSessionListReq* request,
                                        ::IM::GetChatSessionListRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取请求用户ID
            std::string rid = request->request_id();
            std::string uid = request->user_id();

            // 数据库查询用户单聊会话列表
            auto sf_list = _mysql_chat_session->single_chat_session(uid);

            // 取出所有的好友ID，获取用户信息
            std::unordered_set<std::string> users_id_list;
            for (const auto& f : sf_list) 
                users_id_list.insert(f.friend_id);

            std::unordered_map<std::string, UserInfo> user_list;
            if (!GetUserInfo(rid, users_id_list, user_list)) 
            {
                LOG_ERROR("{}: 批量获取用户信息失败!", rid);
                return err_response("批量获取用户信息失败!");
            }

            // 数据库中查询用户群聊会话列表
            auto gc_list = _mysql_chat_session->group_chat_session(uid);

            // 响应
            for (const auto& f : sf_list) // 单聊响应
            {
                auto chat_session_info = response->add_chat_session_info_list();
                chat_session_info->set_single_chat_friend_id(f.friend_id);
                chat_session_info->set_chat_session_id(f.session_id);
                chat_session_info->set_chat_session_name(user_list[f.friend_id].nickname());
                chat_session_info->set_avatar(user_list[f.friend_id].avatar());

                MessageInfo msg;
                if (GetRecentMsg(rid, f.session_id, msg)) 
                    chat_session_info->mutable_prev_message()->CopyFrom(msg); // 获最后一条消息
            }

            for (const auto& f : gc_list) // 群聊响应
            {
                auto chat_session_info = response->add_chat_session_info_list();
                chat_session_info->set_chat_session_id(f.session_id);
                chat_session_info->set_chat_session_name(f.session_name);

                MessageInfo msg;
                if (GetRecentMsg(rid, f.session_id, msg)) 
                    chat_session_info->mutable_prev_message()->CopyFrom(msg); // 获最后一条消息
            }

            response->set_success(true);
        }

        // 创造会话
        virtual void ChatSessionCreate(::google::protobuf::RpcController* controller,
                                        const ::IM::ChatSessionCreateReq* request,
                                        ::IM::ChatSessionCreateRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取会话名称,会话成员
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string ssname = request->chat_session_name();
            
            // 生成会话ID，向数据库添加信息
            std::string ssid = UUID::uuid();
            ChatSession cs(ssid, ssname, ChatSessionType::GROUP);
            if (!_mysql_chat_session->insert(cs)) 
            {
                LOG_ERROR("{}: 向数据库添加会话信息失败: {}", rid, ssname);
                return err_response("向数据库添加会话信息失败!");
            }

            // 添加会话成员
            std::vector<SessionMember> member_list;
            for (int i = 0; i < request->member_id_list_size(); i++) 
            {
                SessionMember csm(ssid, request->member_id_list(i));
                member_list.push_back(csm);
            }

            if (!_mysql_session_member->append(member_list)) {
                LOG_ERROR("{}:  向数据库添加会话成员信息失败: {}", rid, ssname);
                return err_response("向数据库添加会话成员信息失败!");
            }

            // 响应
            response->set_success(true);
            response->mutable_chat_session_info()->set_chat_session_id(ssid);
            response->mutable_chat_session_info()->set_chat_session_name(ssname);
        }

        // 获取会话成员
        virtual void GetChatSessionMember(::google::protobuf::RpcController* controller,
                                            const ::IM::GetChatSessionMemberReq* request,
                                            ::IM::GetChatSessionMemberRsp* response,
                                            ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            // 提取聊天会话ID
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string ssid = request->chat_session_id();

            // 从数据库获取会话成员ID列表
            auto member_id_lists = _mysql_session_member->members(ssid);
            std::unordered_set<std::string> uid_list;
            for (const auto &id : member_id_lists) 
                uid_list.insert(id);

            // 从用户子服务批量获取用户信息
            std::unordered_map<std::string, UserInfo> user_list;
            if (!GetUserInfo(rid, uid_list, user_list)) 
            {
                LOG_ERROR("{}: 从用户子服务获取用户信息失败!", rid);
                return err_response("从用户子服务获取用户信息失败!");
            }

            // 响应
            response->set_success(true);
            for (const auto& it : user_list) 
            {
                auto user_info = response->add_member_info_list();
                user_info->CopyFrom(it.second);
            }
        }

        // 群聊添加成员
        virtual void GroupAddMember(::google::protobuf::RpcController* controller,
                                    const ::IM::GroupAddMemberReq* request,
                                    ::IM::GroupAddMemberRsp* response,
                                    ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            std::string uid = request->user_id();
            std::string rid = request->request_id();
            std::string ssid = request->chat_session_id();

            // 检查 ssid 存在性
            auto cs = _mysql_chat_session->select(ssid);
            if (cs == nullptr) 
            {
                LOG_ERROR("{}: 会话 {} 不存在", rid, ssid);
                return err_response("会话不存在!");
            }

            // mysql添加会话成员
            std::vector<SessionMember> member_list;
            for (int i = 0; i < request->member_id_list_size(); i++) 
            {
                SessionMember csm(ssid, request->member_id_list(i));
                member_list.push_back(csm);
            }

            if (!_mysql_session_member->append(member_list)) 
            {
                LOG_ERROR("{}:  向数据库添加会话成员信息失败: {}", rid, ssid);
                return err_response("向数据库添加会话成员信息失败!");
            }

            // 从用户子服务批量获取用户信息
            std::unordered_set<std::string> uid_list;
            for (int i = 0; i < request->member_id_list_size(); i++) 
                uid_list.insert(request->member_id_list(i));
            
            std::unordered_map<std::string, UserInfo> user_list;
            if (!GetUserInfo(rid, uid_list, user_list)) 
            {
                LOG_ERROR("{}: 从用户子服务获取用户信息失败!", rid);
                return err_response("从用户子服务获取用户信息失败!");
            }

            // 响应            
            response->set_success(true);
            for (const auto& it : user_list) 
            {
                auto user_info = response->add_member_info_list();
                user_info->CopyFrom(it.second);
            }

            MessageInfo msg;
            if (GetRecentMsg(rid, ssid, msg)) 
                response->mutable_chat_session_info()->mutable_prev_message()->CopyFrom(msg); // 获最后一条消息

            response->mutable_chat_session_info()->set_chat_session_id(ssid);
            response->mutable_chat_session_info()->set_chat_session_name(cs->sessionName());
        }

        // 修改群聊名称
        virtual void GroupModifyName(::google::protobuf::RpcController* controller,
                                        const ::IM::GroupModifyNameReq* request,
                                        ::IM::GroupModifyNameRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            std::string uid = request->user_id();
            std::string rid = request->request_id();
            std::string ssid = request->chat_session_id();

            // 检查 ssid 存在性
            auto chatSession = _mysql_chat_session->select(ssid);
            if (chatSession == nullptr) 
            {
                LOG_ERROR("{}: 会话 {} 不存在", rid, ssid);
                return err_response("会话不存在!");
            }

            if (chatSession->sessionType() != ChatSessionType::GROUP)
            {
                LOG_ERROR("{}: 会话 {} 不是群聊", rid, ssid);
                return err_response("会话不是群聊!");
            }

            // 修改群聊名称
            chatSession->sessionName(request->chat_session_name());
            if (!_mysql_chat_session->update(chatSession))
            {
                LOG_ERROR("{}: {} 更新数据库群聊名称失败!", request->request_id(), ssid);
                return err_response("更新群聊名称失败!");
            }

            response->set_success(true);
        }

        // 成员离开群聊
        virtual void MemberLeaveGroup(::google::protobuf::RpcController* controller,
                                        const ::IM::MemberLeaveGroupReq* request,
                                        ::IM::MemberLeaveGroupRsp* response,
                                        ::google::protobuf::Closure* done) override
        {
            brpc::ClosureGuard rpc_guard(done);
            response->set_request_id(request->request_id());

            auto err_response = [response](const std::string &errmsg) {
                response->set_success(false);
                response->set_errmsg(errmsg);
            };

            std::string uid = request->user_id();
            std::string rid = request->request_id();
            std::string ssid = request->chat_session_id();

            // 检查 ssid 存在性
            auto chatSession = _mysql_chat_session->select(ssid);
            if (chatSession == nullptr) 
            {
                LOG_ERROR("{}: 会话 {} 不存在", rid, ssid);
                return err_response("会话不存在!");
            }

            if (chatSession->sessionType() != ChatSessionType::GROUP)
            {
                LOG_ERROR("{}: 会话 {} 不是群聊", rid, ssid);
                return err_response("该会话不是群聊!");
            }

            SessionMember csm(ssid, uid);
            if (!_mysql_session_member->remove(csm))
            {
                LOG_ERROR("{}: {} 数据库删除失败", rid, ssid);
                return err_response("数据库删除失败!");
            }

            response->set_success(true);
        }

    private:
        bool GetRecentMsg(const std::string &rid, const std::string &ssid, MessageInfo &msg) 
        {
            auto channel = _channel_manager->choose(_message_service_name);
            if (!channel) 
            {
                LOG_ERROR("{}: 获取消息子服务信道失败!", rid);
                return false;
            }

            GetRecentMsgReq req;
            GetRecentMsgRsp rsp;
            brpc::Controller cntl;

            req.set_request_id(rid);
            req.set_chat_session_id(ssid);
            req.set_msg_count(1);

            IM::MsgStorageService_Stub stub(channel.get());
            stub.GetRecentMsg(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed()) 
            {
                LOG_ERROR("{}: 消息存储子服务调用失败: {}", rid, cntl.ErrorText());
                return false;
            }

            if (!rsp.success()) 
            {
                LOG_ERROR("{}: 获取会话 {} 最近消息失败: {}", rid, ssid, rsp.errmsg());
                return false;
            }

            if (rsp.msg_list_size() > 0) 
            {
                msg.CopyFrom(rsp.msg_list(0));
                return true;
            }

            return false;
        }

        bool GetUserInfo(const std::string& rid, 
                            const std::unordered_set<std::string>& uid_list,
                            std::unordered_map<std::string, UserInfo>& user_list) 
        {
            auto channel = _channel_manager->choose(_user_service_name);
            if (!channel) 
            {
                LOG_ERROR("{}: 获取用户子服务信道失败!", rid);
                return false;
            }

            GetMultiUserInfoReq req;
            GetMultiUserInfoRsp rsp;
            brpc::Controller cntl;

            req.set_request_id(rid);
            for (auto& id : uid_list)
                req.add_users_id(id);

            IM::UserService_Stub stub(channel.get());
            stub.GetMultiUserInfo(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed()) 
            {
                LOG_ERROR("{}: 用户子服务调用失败: {}", rid, cntl.ErrorText());
                return false;
            }

            if (!rsp.success()) 
            {
                LOG_ERROR("{}: 批量获取用户信息失败: {}", rid, rsp.errmsg());
                return false;
            }

            for (const auto& user_it : rsp.users_info())
                user_list.insert({user_it.first, user_it.second});

            return true;
        }

    private:
        ESUser::ptr _es_user;

        FriendApplyTable::ptr _mysql_apply;
        ChatSessionTable::ptr _mysql_chat_session;
        SessionMemberTable::ptr _mysql_session_member;
        RelationTable::ptr _mysql_relation;

        // rpc
        std::string _user_service_name;
        std::string _message_service_name;
        ServiceManager::ptr _channel_manager;
    };

    // 用户服务类
    class FriendServer
    {
    public:
        using ptr = std::shared_ptr<FriendServer>;
        FriendServer(const Discovery::ptr& dis_client, 
                        const Registry::ptr& reg_client,
                        const std::shared_ptr<elasticlient::Client>& es_client,
                        const std::shared_ptr<odb::core::database>& mysql_client,
                        const std::shared_ptr<brpc::Server>& rpc_server)
                    : _dis_client(dis_client)
                    , _reg_client(reg_client)
                    , _es_client(es_client)
                    , _mysql_client(mysql_client)
                    , _rpc_server(rpc_server)
        {}

        ~FriendServer() = default;

        void start() 
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        Discovery::ptr _dis_client;
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;

        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<odb::core::database> _mysql_client;
    };

    class FriendServiceBuilder
    {
    public:
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
                                    const std::string& user_service_name,
                                    const std::string& message_service_name)
        {
            _user_service_name = user_service_name;
            _message_service_name = message_service_name;
            
            _channel_manager = std::make_shared<ServiceManager>();
            _channel_manager->declared(_user_service_name);
            _channel_manager->declared(message_service_name);
            
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
            if (!_es_client || !_mysql_client || !_channel_manager)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            _rpc_server = std::make_shared<brpc::Server>();

            FriendServiceImpl* friend_service = new FriendServiceImpl(_es_client, _mysql_client,
                                                    _channel_manager, _user_service_name, _message_service_name);

            if (-1 == _rpc_server->AddService(friend_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE))
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

        FriendServer::ptr build()
        {
            if (!_dis_client || !_reg_client || !_rpc_server)
            {
                LOG_ERROR("建造尚未完成!");
                abort();
            }

            return std::make_shared<FriendServer>(_dis_client, _reg_client, 
                                                    _es_client, _mysql_client, _rpc_server);
        }

    private:
        Registry::ptr _reg_client;
        Discovery::ptr _dis_client;
        std::shared_ptr<brpc::Server> _rpc_server;

        std::string _user_service_name;
        std::string _message_service_name;
        ServiceManager::ptr _channel_manager;

        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<odb::core::database> _mysql_client;
    };
}