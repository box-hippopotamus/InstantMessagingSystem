#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>

#include <thread>

int main(int argc, char* argv[])
{
    std::string etcd_host = "http://127.0.0.1:2379";
    // 实例化客户端对象
    etcd::Client client(etcd_host);

    // 获取租约保活对象
    auto keep_alive = client.leasekeepalive(3).get();
    // leaskeepalive设置保活时间，返回 pplx:tack<shared_ptr<KeepAlive>>，相当于一个future对象，get获取内部的智能指针
    auto lease_id = keep_alive->Lease();

    // etcd 新增数据
    auto resp1 = client.put("/service/user", "127.0.0.1:8080", lease_id).get();
    if (!resp1.is_ok()) // 操作是否成功
    {
        std::cout << "error: " << resp1.error_message() << std::endl;
        return -1;
    }

    auto resp2 = client.put("/service/friend", "127.0.0.1:9090", lease_id).get();
    if (!resp2.is_ok()) // 操作是否成功
    {
        std::cout << "error: " << resp2.error_message() << std::endl;
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}