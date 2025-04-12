#include <cstdlib>
#include <iostream>
#include <alibabacloud/core/AlibabaCloud.h>
#include <alibabacloud/core/CommonRequest.h>
#include <alibabacloud/core/CommonClient.h>
#include <alibabacloud/core/CommonResponse.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <string>

// 使用 libcurl 发送 POST HTTP 请求
bool sendRequestWithCurl(const std::string& url) 
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "初始化libcurl失败。" << std::endl;
        return false;
    }

    // 设置 URL - 分离URL和参数
    std::string baseUrl = url;
    std::string postFields = "";
    
    // 找到URL中的参数部分
    size_t pos = url.find('?');
    if (pos != std::string::npos) {
        baseUrl = url.substr(0, pos);
        postFields = url.substr(pos + 1);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str()); // 设置基础URL
    curl_easy_setopt(curl, CURLOPT_POST, 1L); // 指定POST请求
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str()); // 设置POST数据
    curl_easy_setopt(curl, CURLOPT_CAINFO, "./cacert.pem"); // 设置 CA 证书路径
    
    // 发送请求
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "libcurl请求失败: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    // 清理
    curl_easy_cleanup(curl);
    return true;
}

// 从错误信息中提取 URL
std::string extractUrlFromErrorMessage(const std::string& errorMessage) 
{
    std::size_t pos = errorMessage.find("POST ");
    pos = errorMessage.find("http");

    if (pos != std::string::npos) 
        return errorMessage.substr(pos); // 从"http"开始提取
    
    return "";
}

int main(int argc, char** argv) 
{
    AlibabaCloud::InitializeSdk();

    AlibabaCloud::ClientConfiguration configuration("cn-zhangjiakou");
    // specify timeout when create client.
    configuration.setConnectTimeout(1500);
    configuration.setReadTimeout(4000);

    std::string access_key_id = "LTAI5t5uWQw6qW1fdM6EEYPk";
    std::string access_key_secret = "XwUM8TRs7huj9VoPjhh6Lg3IPGsW4N";
    AlibabaCloud::Credentials credential(access_key_id, access_key_secret);

    AlibabaCloud::CommonClient client(credential, configuration);
    AlibabaCloud::CommonRequest request(AlibabaCloud::CommonRequest::RequestPattern::RpcPattern);
    request.setHttpMethod(AlibabaCloud::HttpRequest::Method::Post);
    request.setDomain("dysmsapi.aliyuncs.com");
    request.setVersion("2017-05-25");
    request.setQueryParameter("Action", "SendSms");
    request.setQueryParameter("PhoneNumbers", "15633890522");
    request.setQueryParameter("SignName", "网络聊天室");
    request.setQueryParameter("TemplateCode", "SMS_479980302");
    request.setQueryParameter("TemplateParam", "{\"code\":\"6655\"}");

    // 发送请求
    auto response = client.commonResponse(request);
    if (response.isSuccess()) {
        printf("请求成功。\n");
        printf("结果: %s\n", response.result().payload().c_str());
    } else {
        // 获取错误信息
        std::string errorMessage = response.error().errorMessage();
        
        // 从错误信息中提取 URL
        std::string url = extractUrlFromErrorMessage(errorMessage);
        if (!url.empty()) {

            // 使用 libcurl 重新发送请求
            if (sendRequestWithCurl(url)) {
                printf("使用libcurl成功发送请求。\n");
            } else {
                printf("使用libcurl发送请求失败。\n");
            }
        }
    }

    AlibabaCloud::ShutdownSdk();
    return 0;
}