#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <brpc/channel.h>

#include "logger.hpp"

namespace IM
{
    // 单个rpc服务的信道管理类
    class ServiceChannel
    {
    public:
        using ChannelPtr = std::shared_ptr<brpc::Channel>;
        using ptr = std::shared_ptr<ServiceChannel>;

        ServiceChannel(const std::string& service_name)
            : _service_name(service_name)
            , _index(0)
        {}

        // 服务上线新节点，添加信道
        void append(const std::string& host)
        {
            auto channel = std::make_shared<brpc::Channel>();
            brpc::ChannelOptions options;
            options.connect_timeout_ms = -1; // 空闲超时时间
            options.timeout_ms = -1; // 请求等待超时时间
            options.max_retry = 3; // 请求重试次数
            options.protocol = "baidu_std"; // 序列化协议
            int ret = channel->Init(host.c_str(), &options);
            if (ret == -1)
            {
                LOG_ERROR("{}节点，{}服务，初始化信道失败!", host, _service_name);
                return;
            }

            std::unique_lock<std::mutex> lock(_mtx);
            _hosts[host] = channel;
            _channels.push_back(channel);
        }

        // 服务下线节点，释放信道
        void remove(const std::string& host)
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto it = _hosts.find(host);
            if (it == _hosts.end())
            {
                LOG_WARN("{}节点尝试删除不存在的信道!", host);
                return;
            }

            for (auto channel = _channels.begin(); channel != _channels.end(); ++channel)
            {
                if (*channel == it->second)
                {
                    _channels.erase(channel);
                    break;
                }
            }

            _hosts.erase(it);
        }

        // 通过负载均衡获取一个信道，发起rpc调用
        ChannelPtr choose()
        {
            std::unique_lock<std::mutex> lock(_mtx);
            if (_channels.size() == 0)
            {
                LOG_WARN("当前没有可以提供 {} 服务的节点!", _service_name);
                return ChannelPtr();
            }

            int32_t idx = _index++ % _channels.size();
            return _channels[idx];
        }

    private:
        std::mutex _mtx; // 对信息加锁
        int32_t _index; // 负载空间计数器
        std::string _service_name; // 服务名称
        std::vector<ChannelPtr> _channels; // 该服务的所有提供者的信道
        std::unordered_map<std::string, ChannelPtr> _hosts; // 通过地址找到信道 
    };

    // 所有rpc服务管理类
    class ServiceManager
    {
    public:
        using ptr = std::shared_ptr<ServiceManager>;

        // 声明要管理的服务
        void declared(const std::string& service_name)
        {
            std::unique_lock<std::mutex> lock(_mtx);
            _follow_services.insert(service_name);
        }

        // 选择一个信道提供服务
        ServiceChannel::ChannelPtr choose(const std::string& service_name)
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto sit = _services.find(service_name);
            if (sit == _services.end())
            {
                LOG_WARN("当前没有提供 {} 服务的管理对象!", service_name);
                return ServiceChannel::ChannelPtr();
            }

            return sit->second->choose();
        }

        // 服务上线回调
        void onServiceOnline(const std::string& service_instance, 
                             const std::string& host)
        {
            std::string service_name = getServiceName(service_instance);
            if (!isFollow(service_name))
            {
                LOG_DEBUG("新增 {} 服务不在关注列表中!", service_name);
                return;
            }
            LOG_DEBUG("新增 {} 服务!", service_name);

            ServiceChannel::ptr service;
            {
                std::unique_lock<std::mutex> lock(_mtx);
                // 获取管理对象
                auto sit = _services.find(service_name);
                if (sit == _services.end())
                {
                    service = std::make_shared<ServiceChannel>(service_name);
                    _services[service_name] = service;
                }
                else
                {
                    service = sit->second;
                }
            }

            if (!service)
            {
                LOG_WARN("新增 {} 服务节点失败!", service_name);
                return;
            }

            service->append(host);
        }

        // 服务下线回调
        void onServiceOffline(const std::string& service_instance, 
                              const std::string& host)
        {
            std::string service_name = getServiceName(service_instance);

            if (!isFollow(service_name))
            {
                LOG_DEBUG("移除 {} 服务不在关注列表中!", service_name);
                return;
            }

            ServiceChannel::ptr service;
            {
                std::unique_lock<std::mutex> lock(_mtx);
                // 获取管理对象
                auto sit = _services.find(service_name);
                if (sit == _services.end())
                {
                    LOG_WARN("被删除的 {} 服务中, {} 节点不存在!", service_name, host);
                    return;
                }
                service = sit->second;
            }

            service->remove(host);
        }

    private:
        bool isFollow(const std::string& service_name)
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto fit = _follow_services.find(service_name);
            return fit != _follow_services.end();
        }

        std::string getServiceName(const std::string& service_instance)
        {
            auto pos = service_instance.find_last_of('/');
            if (pos == std::string::npos)
                return service_instance;

            return service_instance.substr(0, pos);
        }

    private:
        std::mutex _mtx;
        std::unordered_set<std::string> _follow_services; 
        std::unordered_map<std::string, ServiceChannel::ptr> _services; 
    };
}