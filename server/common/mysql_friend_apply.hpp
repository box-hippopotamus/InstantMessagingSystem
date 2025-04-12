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
#include "friend_apply.hxx"
#include "friend_apply-odb.hxx"

namespace IM
{
    class FriendApplyTable
    {
    public:
        using ptr = std::shared_ptr<FriendApplyTable>;

        FriendApplyTable(const std::shared_ptr<odb::core::database>& db)
            : _db(db)
        {}

        // 添加好友申请事件
        bool insert(FriendApply& event)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->persist(event);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} : {} 好友申请新增失败: {}", event.userId(), event.peerId(), e.what());
                return false;
            }
            return true;
        }

        // 删除好友申请事件
        bool remove(const std::string& user_id, const std::string& peer_id)
        {
            try {
                using query = odb::query<FriendApply>;
                using result = odb::result<FriendApply>;
                odb::transaction trans(_db->begin());
                _db->erase_query<FriendApply>(query::user_id == user_id
                                                && query::peer_id == peer_id);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} : {} 好友申请删除失败: {}", user_id, peer_id, e.what());
                return false;
            }
            return true;
        }

        bool exists(const std::string& user_id, const std::string& peer_id)
        {
            bool ret;
            try {
                using query = odb::query<FriendApply>;
                using result = odb::result<FriendApply>;
                odb::transaction trans(_db->begin());
                result res(_db->query<FriendApply>(query::user_id == user_id && query::peer_id == peer_id));
                ret = !res.empty();
                trans.commit();
            } catch (std::exception &e) {
                LOG_ERROR("{} : {} 获取好友申请事件失败: {}", user_id, peer_id, e.what());
                return false;
            }
            return ret;
        }

        // 获取好友申请列表
        std::vector<std::string> apply_users(const std::string& user_id)
        {
            std::vector<std::string> ret;
            try {
                using query = odb::query<FriendApply>;
                using result = odb::result<FriendApply>;
                odb::transaction trans(_db->begin());

                result res(_db->query<FriendApply>(query::peer_id == user_id));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(it->userId());
                
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 查询所有好友申请失败: {}", user_id, e.what());
                return {};
            }
            return ret;
        }

    private:
        std::shared_ptr<odb::core::database> _db;
    };
}
