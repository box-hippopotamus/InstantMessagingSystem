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
#include "user.hxx"
#include "user-odb.hxx"

namespace IM
{
    class UserTable
    {
    public:
        using ptr = std::shared_ptr<UserTable>;

        UserTable(const std::shared_ptr<odb::core::database>& db)
            : _db(db)
        {}

        bool insert(const std::shared_ptr<User>& user)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->persist(*user);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 用户新增失败: {}", user->nickname(), e.what());
                return false;
            }
            return true;
        }

        bool update(const std::shared_ptr<User>& user)
        {
            try {
                odb::transaction trans(_db->begin());
                _db->update(*user);
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 用户更新失败: {}", user->nickname(), e.what());
                return false;
            }
            return true;
        }

        std::shared_ptr<User> select_by_nickname(const std::string& nickname)
        {
            std::shared_ptr<User> ret;
            try {
                using query = odb::query<User>;
                using result = odb::result<User>;

                odb::transaction trans(_db->begin());
                ret.reset(_db->query_one<User>(query::nickname == nickname));
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 用户查询失败: {}", nickname, e.what());
            }
            return ret;
        }

        std::shared_ptr<User> select_by_phone(const std::string& phone)
        {
            std::shared_ptr<User> ret;
            try {
                using query = odb::query<User>;
                using result = odb::result<User>;

                odb::transaction trans(_db->begin());
                ret.reset(_db->query_one<User>(query::phone == phone));
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} 手机号查询失败: {}", phone, e.what());
            }
            return ret;
        }

        std::shared_ptr<User> select_by_id(const std::string& user_id)
        {
            std::shared_ptr<User> ret;
            try {
                using query = odb::query<User>;
                using result = odb::result<User>;

                odb::transaction trans(_db->begin());
                ret.reset(_db->query_one<User>(query::user_id == user_id));
                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("{} id查询失败: {}", user_id, e.what());
            }
            return ret;
        }

        std::vector<User> select_by_ids(const std::vector<std::string>& id_list)
        {
            std::cout << id_list.size() << std::endl;

            std::vector<User> ret;
            if (id_list.empty())
                return ret;
            ret.reserve(id_list.size());
            try {
                using query = odb::query<User>;
                using result = odb::result<User>;

                std::stringstream condition;
                condition << "user_id in (";
                for (auto& id : id_list)
                {
                    if (id != id_list.front())
                        condition << ",";
                    condition << "'" << id << "'";
                }
                condition << ")";

                odb::transaction trans(_db->begin());
                result res(_db->query<User>(condition.str()));
                for (result::iterator it(res.begin()); it != res.end(); it++)
                    ret.push_back(*it);

                trans.commit();
            } catch(std::exception& e) {
                LOG_ERROR("id_list 查询失败: {}", e.what());
            }
            return ret;
        }

    private:
        std::shared_ptr<odb::core::database> _db;
    };
}
