#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Value.hpp>

#include <thread>

void callback(const etcd::Response& resp)
{
    if (!resp.is_ok()) // 操作是否成功
    {
        std::cout << "error: " << resp.error_message() << std::endl;
        return;
    }

    for (auto const& ev : resp.events())
    {
        // events 返回 std::vector<Event>
        // Event:
        // event_type  ->  事件类型
        // kv          ->  新值
        // prev_kv     ->  旧值

        if (ev.event_type() == etcd::Event::EventType::PUT)
        {
            std::cout << "服务信息改动:" << std::endl;
            std::cout << "初始值:" << ev.prev_kv().key() << " : " << ev.prev_kv().as_string() << std::endl;
            std::cout << "变化值:" << ev.kv().key() << " : " << ev.kv().as_string() << std::endl;
        }
        else if (ev.event_type() == etcd::Event::EventType::DELETE_)
        {
            std::cout << "服务信息被删除:" << std::endl;
            std::cout << "初始值:" << ev.prev_kv().key() << " : " << ev.prev_kv().as_string() << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    std::string etcd_host = "http://127.0.0.1:2379";

    // 实例化客户端对象
    etcd::Client client(etcd_host);

    // 获取指定键值对信息
    auto resp = client.ls("/service").get();
    //                 获取service目录下的所有信息
    if (!resp.is_ok()) // 操作是否成功
    {
        std::cout << "error: " << resp.error_message() << std::endl;
        return -1;
    }

    int sz = resp.keys().size();
    for (int i = 0; i < sz; i++)
    {
        std::cout << resp.value(i).as_string() << "可提供" << resp.key(i) << "服务!" << std::endl;
    }

    // 实例化键值对监控对象
    auto watcher = etcd::Watcher(client, "/service", callback, true);
    //  callback->监控对象变化时的回调  true->监控目录下所有内容的变化

    std::this_thread::sleep_for(std::chrono::seconds(20));

    return 0;
}