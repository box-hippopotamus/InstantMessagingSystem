#include <sw/redis++/redis++.h>
#include <iostream>

namespace IM
{
    class RedisClientFactory
    {
    public:
        static std::shared_ptr<sw::redis::Redis> create(const std::string& host, int port, int db, bool keep_alive)
        {
            sw::redis::ConnectionOptions opts;
            opts.host = host;
            opts.port = port;
            opts.db = db;
            opts.keep_alive = keep_alive;
            return std::make_shared<sw::redis::Redis>(opts);
        }
    };

    // 管理用户 session
    class Session
    {
    public:
        using ptr = std::shared_ptr<Session>;

        Session(const std::shared_ptr<sw::redis::Redis>& redis_client)
            : _redis_client(redis_client)
        {}

        void append(const std::string& ssid, const std::string& uid)
        {
            _redis_client->set(ssid, uid);
        }

        void remove(const std::string& ssid)
        {
            _redis_client->del(ssid);
        }

        sw::redis::OptionalString uid(const std::string& ssid)
        {
            return _redis_client->get(ssid);
        }

    private:
        std::shared_ptr<sw::redis::Redis> _redis_client;
    };

    // 检测用户是否登录
    class Status
    {
    public:
        using ptr = std::shared_ptr<Status>;

        Status(const std::shared_ptr<sw::redis::Redis>& redis_client)
            : _redis_client(redis_client)
        {}  

        void append(const std::string& uid)
        {
            _redis_client->set(uid, "");
        }

        void remove(const std::string& uid)
        {
            _redis_client->del(uid);
        }

        bool exists(const std::string& uid)
        {
            return _redis_client->get(uid).has_value();
        }

    private:
        std::shared_ptr<sw::redis::Redis> _redis_client;
    };

    // 验证码管理
    class Codes
    {
    public:
        using ptr = std::shared_ptr<Codes>;

        Codes(const std::shared_ptr<sw::redis::Redis>& redis_client)
            : _redis_client(redis_client)
        {}

        void append(const std::string& cid, const std::string& code)
        {
            _redis_client->set(cid, code, std::chrono::seconds(300)); // 五分钟有效时间
        }

        void remove(const std::string& cid)
        {
            _redis_client->del(cid);
        }

        sw::redis::OptionalString code(const std::string &cid) 
        {
            return _redis_client->get(cid);
        }
        
    private:
        std::shared_ptr<sw::redis::Redis> _redis_client;
    };
}