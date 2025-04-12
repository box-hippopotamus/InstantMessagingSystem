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
#include "message.hxx"
#include "message-odb.hxx"

namespace IM
{
    class MessageTable
    {
    public:
        using ptr = std::shared_ptr<MessageTable>;

        MessageTable(const std::shared_ptr<odb::core::database>& db)
            : _db(db)
        {}

        bool insert(Message& message)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->persist(message);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 消息新增失败: {}", message.messageId(), e.what());
                return false;
            }
            return true;
        }

        bool remove(const std::string& session_id)
        {
            try {
                using query = odb::query<Message>;
                using result = odb::result<Message>;
                odb::transaction trans(_db->begin());
                _db->erase_query<Message>(query::session_id == session_id);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 消息删除失败: {}", session_id, e.what());
                return false;
            }
            return true;
        }

        std::vector<Message> recent(const std::string& session_id, int count)
        {
            std::vector<Message> ret;
            ret.reserve(count);
            try {
                using query = odb::query<Message>;
                using result = odb::result<Message>;

                std::stringstream condition;
                condition << "session_id='" << session_id << "' ";
                condition << "order by create_time desc limit " << count;

                odb::transaction trans(_db->begin());
                result res(_db->query<Message>(condition.str()));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(*it);
                
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 近期消息查询失败: {}", session_id, e.what());
            }
            std::reverse(ret.begin(), ret.end());
            return ret;
        }

        std::vector<Message> range(const std::string& session_id, 
                                    boost::posix_time::ptime start, 
                                    boost::posix_time::ptime end)
        {
            std::vector<Message> ret;
            try {
                using query = odb::query<Message>;
                using result = odb::result<Message>;
                odb::transaction trans(_db->begin());
                result res(_db->query<Message>(query::session_id == session_id
                                                && query::create_time >= start 
                                                && query::create_time <= end));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(*it);
                
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 区间消息查询失败: {}", session_id, e.what());
            }
            return ret;
        }

    private:
        std::shared_ptr<odb::core::database> _db;
    };
}
