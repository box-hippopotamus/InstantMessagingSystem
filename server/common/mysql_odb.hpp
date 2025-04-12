#pragma once

#include <string>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>

namespace IM
{
    class ODBFactory
    {
    public:
        static std::shared_ptr<odb::core::database> create(
            const std::string& user,
            const std::string& password,
            const std::string& host,
            const std::string& db,
            const std::string& charset,
            int port,
            int conn_pool_count)
        {
            // 构造连接池工厂   
            std::unique_ptr<odb::mysql::connection_pool_factory> cpf(
                new odb::mysql::connection_pool_factory(conn_pool_count, 0));

            // 构造数据库对象
            return std::make_shared<odb::mysql::database>(user, password, 
                db, host, port, "", charset, 0, std::move(cpf));
        }
    };
}
