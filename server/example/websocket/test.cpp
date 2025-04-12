#include <iostream>

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

using server_t = websocketpp::server<websocketpp::config::asio>;

// 连接建立回调
void onOpen(websocketpp::connection_hdl hdl)
{
    std::cout << "websocket 长链接建立成功!" << std::endl;
}

// 连接关闭回调
void onClose(websocketpp::connection_hdl hdl)
{
    std::cout << "websocket 长链接断开!" << std::endl;
}

// 消息到达回调
void onMessage(server_t* server, websocketpp::connection_hdl hdl, server_t::message_ptr msg)
{
    std::cout << "websocket 收到消息: " << std::endl;

    std::string body = msg->get_payload();// 获取载荷

    auto conn = server->get_con_from_hdl(hdl); // 获取连接

    conn->send(body + " - echo", websocketpp::frame::opcode::value::text);
}


int main(int argc, char* argv[])
{
    // 初始化实例对象
    server_t server;

    // 关闭日志输出
    server.set_access_channels(websocketpp::log::alevel::none);

    // 初始化asio
    server.init_asio();

    // 设置回调
    server.set_open_handler(onOpen);
    server.set_close_handler(onClose);

    auto func = std::bind(onMessage, &server, std::placeholders::_1, std::placeholders::_2);

    server.set_message_handler(func);

    // 启用地址复用
    server.set_reuse_addr(true);

    // 设置端口
    server.listen(6666);

    // 开始监听
    server.start_accept();

    // 启动服务器
    server.run();

    return 0;
}