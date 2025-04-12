#include <iostream>

#include <butil/logging.h>
#include <brpc/channel.h>

#include <thread>

#include "test.pb.h"


void callback(brpc::Controller* cntl, ::example::EchoResponse* response)
{
    std::cout << response->message() << std::endl;
    delete cntl;
    delete response;
}

int main(int argc, char* argv[])
{
    // 构造channel信道，连接服务器
    brpc::ChannelOptions options;
    options.connect_timeout_ms = -1; // 空闲超时时间
    options.timeout_ms = -1; // 请求等待超时时间
    options.max_retry = 3; // 请求重试次数
    options.protocol = "baidu_std"; // 序列化协议

    brpc::Channel channel;
    int ret = channel.Init("127.0.0.1:6666", &options);
    if (ret == -1)
    {
        std::cout << "创建信道失败!" << std::endl;
        return -1;
    }

    // 构造Echoservice_Stub对象，进行rpc调用
    example::EchoService_Stub stub(&channel); // 客户端，需要传入信道channel

    // 调用
    example::EchoRequest req;
    req.set_message("hello world");

    brpc::Controller* cntl = new brpc::Controller();
    example::EchoResponse* rsp = new example::EchoResponse();  // new出对象，防止回调时已经被释放

    auto clusure = google::protobuf::NewCallback(callback, cntl, rsp); // 后续传入函数参数

    stub.Echo(cntl, &req, rsp, clusure); // 第四个参数传入nullptr表示同步调用
    if (cntl->Failed())
    {
        std::cout << "rpc调用失败: " << cntl->ErrorText() << std::endl;
        return -1;
    }

    std::cout << rsp->message() << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}