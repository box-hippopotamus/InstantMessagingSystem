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
#include "relation.hxx"
#include "relation-odb.hxx"

namespace IM
{
    class RelationTable
    {
    public:
        using ptr = std::shared_ptr<RelationTable>;

        RelationTable(const std::shared_ptr<odb::core::database>& db)
            : _db(db)
        {}

        // 新增关系
        bool insert(const std::string& user_id, const std::string& peer_id)
        {
            try {
                odb::transaction trans(_db->begin());
                Relation user_to_peer(user_id, peer_id);
                Relation peer_to_user(peer_id, user_id);
                _db->persist(user_to_peer);
                _db->persist(peer_to_user);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} : {} 好友新增失败: {}", user_id, peer_id, e.what());
                return false;
            }
            return true;
        }

        // 移除关系
        bool remove(const std::string& user_id, const std::string& peer_id)
        {
            try {
                using query = odb::query<Relation>;
                using result = odb::result<Relation>;
                odb::transaction trans(_db->begin());
                _db->erase_query<Relation>(query::user_id == user_id 
                                            && query::peer_id == peer_id);
                _db->erase_query<Relation>(query::peer_id == user_id 
                                            && query::user_id == peer_id);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} : {} 好友删除失败: {}", user_id, peer_id, e.what());
                return false;
            }
            return true;
        }

        // 判断关系是否存在
        bool exists(const std::string& user_id, const std::string& peer_id)
        {               
            using query = odb::query<Relation>;
            using result = odb::result<Relation>;
            bool ret;
            try {
                odb::transaction trans(_db->begin());
                result res = _db->query<Relation>(query::user_id == user_id 
                                            && query::peer_id == peer_id);
                ret = !res.empty();
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} : {} 查询好友失败: {}", user_id, peer_id, e.what());
                return false;
            }
            return ret;
        }

        // 获取所有好友id
        std::vector<std::string> friends(const std::string& user_id)
        {               
            std::vector<std::string> ret;
            try {
                using query = odb::query<Relation>;
                using result = odb::result<Relation>;
                odb::transaction trans(_db->begin());

                result res(_db->query<Relation>(query::user_id == user_id));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(it->peerId());
                
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{}: 查询所有好友失败: {}", user_id, e.what());
                return {};
            }
            return ret;
        }

    private:
        std::shared_ptr<odb::core::database> _db;
    };
}
