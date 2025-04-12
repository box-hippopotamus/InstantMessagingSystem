#pragma once

#include <memory>
#include <functional>

#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Value.hpp>

#include "logger.hpp"

namespace IM
{
    // 服务注册客户端
    class Registry
    {
    public:
        using ptr = std::shared_ptr<Registry>;

        Registry(const std::string& host)
            : _client(std::make_shared<etcd::Client>(host))
            , _keep_alive(_client->leasekeepalive(3).get())
            , _lease_id(_keep_alive->Lease())
        {}

        ~Registry()
        {
            _keep_alive->Cancel();
        }

        // 新增数据 -> 注册一个服务
        bool registry(const std::string& key, const std::string& val)
        {
            auto resp = _client->put(key, val, _lease_id).get();
            if (!resp.is_ok()) // 操作是否成功
            {
                LOG_ERROR("注册数据失败: {}", resp.error_message());
                return false;
            }

            return true;
        }

    private:
        std::shared_ptr<etcd::Client> _client; // 客户端
        std::shared_ptr<etcd::KeepAlive> _keep_alive; // 租约对象
        uint64_t _lease_id; // 租约id
    };

    // 服务发现客户端
    class Discovery
    {
    public:
        using ptr = std::shared_ptr<Discovery>;
        //                                              服务        主机  -> 通知某个服务上线了某一台主机
        using NotifyCallback = std::function<void(std::string, std::string)>;

        Discovery(const std::string& host,  // 注册中心地址(etcd服务端)
                  const std::string& basedir, // 根目录
                 const NotifyCallback& put_cb, // 服务上线通知回调 -> 服务上下线表现为键值对的增删
                 const NotifyCallback& del_cb) // 服务下线通知回调
                : _put_cb(put_cb)
                , _del_cb(del_cb)
                , _client(std::make_shared<etcd::Client>(host))
        {
            // 服务发现，获取数据
            auto resp = _client->ls(basedir).get();
            if (!resp.is_ok())
            {
                LOG_ERROR("获取服务信息失败: {}", resp.error_message());
            }
        
            int sz = resp.keys().size();
            for (int i = 0; i < sz; i++)
            {
                if (_put_cb) 
                    _put_cb(resp.key(i), resp.value(i).as_string());
            }
        
            // 实例化键值对监控对象
            auto cb = [this](const etcd::Response& resp){
                callback(resp);
            };

            _watcher = std::make_shared<etcd::Watcher>(*_client.get(), basedir, cb, true);
        }

    private:
        void callback(const etcd::Response& resp)
        {
            if (!resp.is_ok())
            {
                LOG_ERROR("错误的事件通知: {}", resp.error_message());
                return;
            }
        
            for (auto const& ev : resp.events())
            {
                if (ev.event_type() == etcd::Event::EventType::PUT)
                {
                    if (_put_cb) 
                        _put_cb(ev.kv().key(), ev.kv().as_string());

                    LOG_DEBUG("新增服务: {}", ev.kv().key());
                }
                else if (ev.event_type() == etcd::Event::EventType::DELETE_)
                {
                    if (_del_cb)
                        _del_cb(ev.prev_kv().key(), ev.prev_kv().as_string());

                    LOG_DEBUG("下线服务: {}", ev.prev_kv().key());
                }
            }
        }

    private:
        NotifyCallback _put_cb;
        NotifyCallback _del_cb;
        std::shared_ptr<etcd::Client> _client; // 客户端
        std::shared_ptr<etcd::Watcher> _watcher; // 事件监控对象
    };
}