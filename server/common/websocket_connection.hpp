#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

#include <unordered_map>
#include <mutex>

namespace IM
{
    class Connection
    {
    public:
        using ptr = std::shared_ptr<Connection>;
        using server_t = websocketpp::server<websocketpp::config::asio>;

        struct Client 
        {
            Client(const std::string& u, const std::string& s)
                : uid(u)
                , ssid(s)
            {}

            std::string uid;
            std::string ssid;
        };

        Connection() = default;

        ~Connection() = default;

        void insert(const server_t::connection_ptr& conn, 
                     const std::string& uid, const std::string& ssid) 
        {
            std::unique_lock<std::mutex> lock(_mtx);
            _uid_connections.insert(std::make_pair(uid, conn));
            _conn_clients.insert(std::make_pair(conn, Client(uid, ssid)));
            LOG_DEBUG("{}: 新增长连接用户信息: {}, {}", (size_t)conn.get(), uid, ssid);
        }

        server_t::connection_ptr connection(const std::string& uid) 
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto it = _uid_connections.find(uid);
            if (it == _uid_connections.end()) 
            {
                LOG_ERROR("{}: 未找到长连接对应客户端信息!", uid);
                return server_t::connection_ptr();
            }

            return it->second;
        }

        bool client(const server_t::connection_ptr& conn, std::string& uid, std::string& ssid) 
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto it = _conn_clients.find(conn);
            if (it == _conn_clients.end()) {
                LOG_ERROR("{}: 未找到长连接对应客户端信息!", (size_t)conn.get());
                return false;
            }
            uid = it->second.uid;
            ssid = it->second.ssid;
            return true;
        }

        void remove(const server_t::connection_ptr& conn) 
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto it = _conn_clients.find(conn);
            if (it == _conn_clients.end()) 
            {
                LOG_ERROR("{}: 未找到长连接对应客户端信息!", (size_t)conn.get());
                return;
            }

            _uid_connections.erase(it->second.uid);
            _conn_clients.erase(it);
        }

    private:
        std::mutex _mtx;
        std::unordered_map<std::string, server_t::connection_ptr> _uid_connections;
        std::unordered_map<server_t::connection_ptr, Client> _conn_clients;
    };
}