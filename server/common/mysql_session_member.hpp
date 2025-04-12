#pragma once

#include <string>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>

#include <gflags/gflags.h>

#include "mysql_odb.hpp"
#include "logger.hpp"
#include "session_member.hxx"
#include "session_member-odb.hxx"

namespace IM
{
    class SessionMemberTable
    {
    public:
        using ptr = std::shared_ptr<SessionMemberTable>;

        SessionMemberTable(const std::shared_ptr<odb::core::database>& db)
            : _db(db)
        {}

        // 添加会话成员
        bool append(SessionMember& session_member)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->persist(session_member);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: {} 会话成员新增失败: {}", session_member.sessionId(), session_member.userId(), e.what());
                return false;
            }
            return true;
        }

        // 批量添加会话成员
        bool append(std::vector<SessionMember>& session_members)
        {
            try {
                odb::transaction trans(_db->begin());
                for (auto& session_member : session_members)
                    _db->persist(session_member);

                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("多会话成员新增失败: {}", e.what());
                return false;
            }
            return true;
        }

        // 移除会话成员
        bool remove(SessionMember& session_member)
        {
            try {
                odb::transaction trans(_db->begin());
                using query = odb::query<SessionMember>;
                using result = odb::result<SessionMember>;
                _db->erase_query<SessionMember>(query::session_id == session_member.sessionId()
                                                && query::user_id == session_member.userId());
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: {} 会话成员删除失败: {}", session_member.sessionId(), session_member.userId(), e.what());
                return false;
            }
            return true;
        }

        // 移除所有会话成员
        bool remove(const std::string& session_id)
        {
            try {
                odb::transaction trans(_db->begin());
                using query = odb::query<SessionMember>;
                using result = odb::result<SessionMember>;
                _db->erase_query<SessionMember>(query::session_id == session_id);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 全部会话成员删除失败: {}", session_id, e.what());
                return false;
            }
            return true;
        }
        
        // 获取所有会话成员id
        std::vector<std::string> members(const std::string& session_id)
        {
            std::vector<std::string> ret;
            try {
                odb::transaction trans(_db->begin());
                using query = odb::query<SessionMember>;
                using result = odb::result<SessionMember>;

                result res(_db->query<SessionMember>(query::session_id == session_id));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(it->userId());

                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 获取会话成员失败: {}", session_id, e.what());
                return {};
            }
            return ret;
        }

    private:
        std::shared_ptr<odb::core::database> _db;
    };
}
