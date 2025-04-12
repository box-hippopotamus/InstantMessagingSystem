#pragma once

#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <openssl/ssl.h>
#include <openssl/opensslv.h>

#include <string>
#include <memory>
#include <thread>
#include <functional>

#include "logger.hpp"

namespace IM
{
    class MQClient
    {
    public:
        using ptr = std::shared_ptr<MQClient>;
        using MessageCallback = std::function<void(const char*, size_t)>;

        MQClient(const std::string& user, const std::string& password, const std::string& host)
        {
            _loop = EV_DEFAULT;
            _handler = std::make_unique<AMQP::LibEvHandler>(_loop);

            std::string url = "amqp://" + user + ":" + password + "@" + host + "/";
            AMQP::Address address(url);
            _connection = std::make_unique<AMQP::TcpConnection>(_handler.get(), address);
            _channel = std::make_unique<AMQP::TcpChannel>(_connection.get());

            _loop_thread = std::thread([this](){
                ev_run(_loop, 0); // 单独维护一个线程跑事件循环
            });
        }
        
        ~MQClient()
        {
            ev_async_init(&_async_watcher, watcher_callback); 
            // 初始化监视器，绑定回调 watcher_callback

            ev_async_start(_loop, &_async_watcher);
            // 将 async_watcher 注册到事件循环 loop 中。
            // 这样，async_watcher 就可以接收来自其他线程的信号。

            ev_async_send(_loop, &_async_watcher); 
            // 向事件循环发送信号，触发 async_watcher， 进一步触发回调函数

            _loop_thread.join();
        }

        void declareComponents(const std::string& exchange, 
                                const std::string& queue, 
                                const std::string& routing_key = "routing_key", 
                                AMQP::ExchangeType exchange_type = AMQP::ExchangeType::direct)
        {
            // 声明交换机
            _channel->declareExchange(exchange, exchange_type)
                    .onError([exchange](const char* message){
                        LOG_ERROR("声明 {} 交换机失败: {}", exchange, message);
                    })
                    .onSuccess([exchange](){
                        LOG_INFO("声明 {} 交换机成功!", exchange);
                    });

            // 声明队列
            _channel->declareQueue(queue)
                    .onError([queue](const char* message){
                        LOG_ERROR("声明 {} 队列失败: {}", queue, message);
                    })
                    .onSuccess([queue](){
                        LOG_INFO("声明 {} 队列成功!", queue);
                    });

            // 绑定交换机与队列
            _channel->bindQueue(exchange, queue, routing_key)
                    .onError([exchange, queue](const char* message){
                        LOG_ERROR("{} 与 {} 绑定失败: {}", exchange, queue, message);
                    })
                    .onSuccess([exchange, queue](){
                        LOG_INFO("{} 与 {} 绑定成功!", exchange, queue);
                    });
        }

        bool publish(const std::string& exchange, 
                        const std::string& msg, 
                        const std::string& routing_key = "routing_key")
        {
            bool ret = _channel->publish(exchange, routing_key, msg);
            if (!ret) LOG_ERROR("{} 消息发送失败!", routing_key);

            return ret;
        }

        void consume(const std::string& queue, const MessageCallback& cb)
        {
            // 订阅服务
            _channel->consume(queue)
                    .onReceived([this, cb](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered){
                        cb(message.body(), message.bodySize());
                        _channel->ack(deliveryTag);
                    })
                    .onError([queue](const char* message){
                        LOG_ERROR("{} 订阅失败: {}", queue, message);
                    });
        }

    private:
        static void watcher_callback(struct ev_loop* loop, ev_async* watcher, int32_t revents)
        {
            //    被中断的循环  中断所有
            ev_break(loop, EVBREAK_ALL);
        }

    private:
        struct ev_async _async_watcher; // 监视器，用于跨线程通知事件循环
        struct ev_loop* _loop;
        std::unique_ptr<AMQP::LibEvHandler> _handler;
        std::unique_ptr<AMQP::TcpConnection> _connection;
        std::unique_ptr<AMQP::TcpChannel> _channel;
        std::thread _loop_thread;
    };
}