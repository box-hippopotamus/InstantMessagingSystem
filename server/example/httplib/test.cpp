#include <iostream>
#include "../../common/httplib.h"

void handler(const httplib::Request& req, httplib::Response& rsp)
{
    std::cout << req.method << std::endl;
    std::cout << req.path << std::endl;

    for (auto header : req.headers)
    {
        std::cout << header.first << " : " << header.second << std::endl;
    }

    rsp.status = 200; // 响应码，默认200
    std::string body = "<html><body><h1>hello cpp-httplib!</h1></body></html>";
    rsp.set_content(body, "text/html");
}

int main(int argc, char* argv[])
{
    // 实例化服务器对象
    httplib::Server server;

    // 注册回调和
    server.Get("/hi", handler);

    // 启动服务器
    server.listen("0.0.0.0", 6666);

    return 0;
}