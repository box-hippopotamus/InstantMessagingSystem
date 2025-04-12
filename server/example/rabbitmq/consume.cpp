#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <openssl/ssl.h>
#include <openssl/opensslv.h>

//                                                                     消息标识
void MessageCb(AMQP::TcpChannel& channel, const AMQP::Message& message, uint64_t deliveryTag, bool redelivered)
{
    std::string msg;
    //        body返回c风格字符串char*
    msg.assign(message.body(), message.bodySize());
    std::cout << msg << std::endl;
    channel.ack(deliveryTag); // 消息确认
}

int main()
{
    // 实例化底层网络通信
    auto* loop = EV_DEFAULT;

    // 实例化 libevhandler句柄 -> 将AMQP框架与事件监控关联
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

    auto cb = [&channel](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered){
        MessageCb(channel, message, deliveryTag, redelivered);
    };

    // 订阅服务                                订阅队列       订阅标识
    channel.consume("test-queue", "consume-tag")
            .onReceived(cb)
            .onError([](const char* message){
                    std::cout << "订阅失败,错误信息: " << message << std::endl;
                    exit(0);
            });

    // 启动底层网络通信框架，开启事件循环
    ev_run(loop, 0);

    return 0;
}