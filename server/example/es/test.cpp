#include <iostream>
#include <elasticlient/client.h>
#include <cpr/cpr.h>

int main(int argc, char* argv[])
{
    // 构造es客户端                 可以传入多个地址，形成分布式服务
    elasticlient::Client client({"http://127.0.0.1:9200/"}); // 不能漏掉尾部的 /

    try
    {
        // 发起搜索请求             索引名称  文档类型          请求体
        auto rsp = client.search("user", "_doc", "{\"query\": {\"match_all\": {}}}");

        // 输出响应状态码
        std::cout << rsp.status_code << std::endl;
        std::cout << rsp.text << std::endl;
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    

    return 0;
}