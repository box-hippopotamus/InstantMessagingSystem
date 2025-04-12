#include <iostream>
#include <sw/redis++/redis.h>
#include <gflags/gflags.h>
#include <thread>

DEFINE_string(ip, "127.0.0.1", "服务器的监听地址");
DEFINE_int32(port, 6379, "服务器的监听端口");
DEFINE_int32(db, 0, "库的编号");
DEFINE_bool(keep_alive, true, "是否进行长链接保活");

void add_string(sw::redis::Redis& client)
{
    client.set("id-1", "val-1");
    client.set("id-2", "val-2");
    client.set("id-3", "val-3");
    client.set("id-4", "val-4");

    client.del("id-3");

    auto user1 = client.get("id-1");

    if (user1) // 返回值是一个optionalstring
        std::cout << *user1 << std::endl;

    auto user2 = client.get("id-2");
    if (user2) // 返回值是一个optionalstring
        std::cout << *user2 << std::endl;

    auto user3 = client.get("id-3");
    if (user3) // 返回值是一个optionalstring
        std::cout << *user3 << std::endl;

    auto user4 = client.get("id-4");
    if (user4) // 返回值是一个optionalstring
        std::cout << *user4 << std::endl;

}

int main(int argc, char* argv[])
{
    // 实例化redis对象，连接服务器
    sw::redis::ConnectionOptions opts;
    opts.host = FLAGS_ip;
    opts.port = FLAGS_port;
    opts.db = FLAGS_db;
    opts.keep_alive = FLAGS_keep_alive;

    sw::redis::Redis client(opts);
    // 添加/删除/获取键值对
    add_string(client);

    // 控制数据有效时间
    client.set("expire-1", "expire", std::chrono::seconds(5)); // 五秒内有效
    auto ret = client.get("expire-1");
    if (ret)
        std::cout << *ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(6));

    ret = client.get("expire-1");
    if (ret)
        std::cout << *ret << std::endl;
    else
        std::cout << "无结果" << std::endl;

    
    // 列表操作


    return 0;
}