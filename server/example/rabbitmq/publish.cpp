#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <openssl/ssl.h>
#include <openssl/opensslv.h>

// AMQP-CPP 只负责应用层的amqp协议封装，而不实现网络层
// 为此要专门引入 livev 这个网络库，来配合 AMQP-CPP
// AMQP-CPP 为几个常用的C++网络库，做了适配
// 因此可以直接把 livev 的事件循环注册到 AMQP 的句柄 handler
// 当事件循环注册完毕，确定服务器地址，就可以获取到信道对象
// 通过信道完成 RabbitMQ 的各类操作

// 初始化：
// AMQP-CPP通过LibEvHandler将socket文件描述符注册到libev。
// libev监听这些socket的可读/可写事件。
//
// 发送消息：
// channel.publish将消息写入AMQP-CPP的缓冲区。
// AMQP-CPP通过LibEvHandler通知libev监听socket的可写事件。
// 当socket可写时，libev触发回调，AMQP-CPP将缓冲区数据发送到网络。
//
// 接收响应：
// libev监听socket可读事件，数据到达时触发回调。
// AMQP-CPP解析AMQP帧，若为声明交换机的响应，则调用onSuccess或onError

int main()
{
    // 实例化底层网络通信
    auto* loop = EV_DEFAULT;

    // 实例化 libevhandler句柄 -> 将AMQP框架与EV事件监控关联
    AMQP::LibEvHandler handler(loop);

    // 实例化网络连接对象
    AMQP::Address address("amqp://root:123456@localhost:5672/");
    AMQP::TcpConnection connection(&handler, address);

    // 实例化信道对象
    AMQP::TcpChannel channel(&connection);

    // 声明交换机                              交换机名称                    交换机类型    标志位
    channel.declareExchange("test-exchange", AMQP::ExchangeType::direct, 0)
            .onError([](const char* message){
                    std::cout << "交换机创建失败,错误信息: " << message << std::endl;
                    exit(0);
            })
            .onSuccess([](){
                std::cout << "交换机创建成功!"<< std::endl;
            });

    // 声明队列
    channel.declareQueue("test-queue")
            .onError([](const char* message){
                    std::cout << "队列创建失败,错误信息: " << message << std::endl;
                    exit(0);
            })
            .onSuccess([](){
                std::cout << "队列创建成功!"<< std::endl;
            });

    // 绑定交换机与队列                              交换机名         队列名         route key
    channel.bindQueue("test-exchange", "test-queue", "test-queue-key")
            .onError([](const char* message){
                    std::cout << "绑定失败,错误信息: " << message << std::endl;
                    exit(0);
            })
            .onSuccess([](){
                std::cout << "绑定成功!"<< std::endl;
            });


    // 向交换机发布消息
    for (int i = 0; i < 10; i++)
    {
        std::string msg;
        msg = "hello " + std::to_string(i);

        // 向指定交换机的指定key发送msg消息
        bool ret = channel.publish("test-exchange", "test-queue-key", msg);
        if (!ret)
            std::cout << "发送失败!" << std::endl;
    }

    // 启动底层网络通信框架，开启事件循环
    ev_run(loop, 0);

    return 0;
}