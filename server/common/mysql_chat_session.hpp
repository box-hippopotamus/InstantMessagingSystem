#pragma once

#include <string>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <gflags/gflags.h>

#include "mysql_odb.hpp"
#include "logger.hpp"
#include "chat_session.hxx"
#include "chat_session-odb.hxx"

namespace IM
{
    class ChatSessionTable
    {
    public:
        using ptr = std::shared_ptr<ChatSessionTable>;

        ChatSessionTable(const std::shared_ptr<odb::core::database>& db)
            : _db(db)
        {}

        // 新增会话
        bool insert(ChatSession& chat_session)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->persist(chat_session);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 会话新增失败: {}", chat_session.sessionName(), e.what());
                return false;
            }
            return true;
        }

        // 删除会话
        bool remove(const std::string& session_id) 
        {
            try {
                using query = odb::query<ChatSession>;
                using result = odb::result<ChatSession>;
                using mquery = odb::query<SessionMember>;

                odb::transaction trans(_db->begin());
                _db->erase_query<ChatSession>(query::session_id == session_id);
                _db->erase_query<SessionMember>(mquery::session_id == session_id);
                trans.commit();
            } catch (std::exception &e) {
                LOG_ERROR("{} 会话删除失败: {}", session_id, e.what());
                return false;
            }
            return true;
        }

        // 更新会话
        bool update(const std::shared_ptr<ChatSession>& chat_session)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->update(*chat_session);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 会话更新失败: {}", chat_session->sessionName(), e.what());
                return false;
            }
            return true;
        }
        
        // 单聊删除
        bool remove(const std::string& user_id, const std::string& peer_id) 
        {
            try {
                using query = odb::query<SingleChatSession>;
                using result = odb::result<SingleChatSession>;
                using cquery = odb::query<ChatSession>;
                using mquery = odb::query<SessionMember>;

                odb::transaction trans(_db->begin());
                auto res = _db->query_one<SingleChatSession>(
                    query::smb1::user_id == user_id && 
                    query::smb2::user_id == peer_id && 
                    query::css::session_type == ChatSessionType::SINGLE);

                std::string session_id = res->session_id;
                _db->erase_query<ChatSession>(cquery::session_id == session_id);
                _db->erase_query<SessionMember>(mquery::session_id == session_id);
                trans.commit();
            } catch (std::exception &e) {
                LOG_ERROR("{} : {} 删除会话失败: {}", user_id, peer_id, e.what());
                return false;
            }
            return true;
        }

        // 获取详细会话信息
        std::shared_ptr<ChatSession> select(const std::string& session_id)
        {
            std::shared_ptr<ChatSession> ret;
            try {
                using query = odb::query<ChatSession>;
                using result = odb::result<ChatSession>;
                odb::transaction trans(_db->begin());
                ret.reset(_db->query_one<ChatSession>(query::session_id == session_id));
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 获取会话信息失败: {}", session_id, e.what());
                return nullptr;
            }
            return ret;
        }

        // 获取单聊会话信息
        std::vector<SingleChatSession> single_chat_session(const std::string& user_id)
        {
            std::vector<SingleChatSession> ret;
            try {
                using query = odb::query<SingleChatSession>;
                using result = odb::result<SingleChatSession>;
                odb::transaction trans(_db->begin());

                result res(_db->query<SingleChatSession>(query::css::session_type == ChatSessionType::SINGLE
                                                            && query::smb1::user_id == user_id 
                                                            && query::smb2::user_id != query::smb1::user_id));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(*it);
                
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 获取单聊会话失败: {}", user_id, e.what());
                return {};
            }
            return ret;
        }

        // 获取群聊会话信息
        std::vector<GroupChatSession> group_chat_session(const std::string& user_id)
        {
            std::vector<GroupChatSession> ret;
            try {
                using query = odb::query<GroupChatSession>;
                using result = odb::result<GroupChatSession>;
                odb::transaction trans(_db->begin());

                result res(_db->query<GroupChatSession>(query::css::session_type == ChatSessionType::GROUP
                                                            && query::smb::user_id == user_id));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(*it);
                
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 获取群聊会话失败: {}", user_id, e.what());
                return {};
            }
            return ret;
        }

    private:
        std::shared_ptr<odb::core::database> _db;
    };
}
