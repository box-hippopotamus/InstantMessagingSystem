#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <alibabacloud/core/AlibabaCloud.h>
#include <alibabacloud/core/CommonRequest.h>
#include <alibabacloud/core/CommonClient.h>
#include <alibabacloud/core/CommonResponse.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <string>

#include "logger.hpp"

namespace IM
{
    class DMSClient
    {
    public:
        using ptr = std::shared_ptr<DMSClient>;

        DMSClient(const std::string& access_key_id, const std::string& access_key_secret, const std::string& ca_path)
            : _ca_path(ca_path)
        {
            AlibabaCloud::InitializeSdk();

            AlibabaCloud::ClientConfiguration configuration("ap-southeast-1");
            configuration.setConnectTimeout(1500);
            configuration.setReadTimeout(4000);
        
            AlibabaCloud::Credentials credential(access_key_id, access_key_secret);
            _client = std::make_unique<AlibabaCloud::CommonClient>(credential, configuration);
        }

        ~DMSClient()
        {
            AlibabaCloud::ShutdownSdk();
        }

        bool send(const std::string& phone, const std::string& code)
        {
            AlibabaCloud::CommonRequest request(AlibabaCloud::CommonRequest::RequestPattern::RpcPattern);
            request.setHttpMethod(AlibabaCloud::HttpRequest::Method::Post);
            request.setDomain("dysmsapi.aliyuncs.com");
            request.setVersion("2017-05-25");
            request.setQueryParameter("Action", "SendSms");
            request.setQueryParameter("PhoneNumbers", phone);
            request.setQueryParameter("SignName", "网络聊天室");
            request.setQueryParameter("TemplateCode", "SMS_479980302");
            std::string total_code = "{\"code\":\"" + code + "\"}";
            request.setQueryParameter("TemplateParam", total_code);
        
            auto response = _client->commonResponse(request);
            if (!response.isSuccess()) 
            {
                std::string errorMessage = response.error().errorMessage();
                std::string url = extractUrlFromErrorMessage(errorMessage); // 从错误信息中提取 URL
                if (url.empty() || !sendRequestWithCurl(url)) 
                {
                    LOG_ERROR("短信验证码请求失败: {}", errorMessage);
                    return false;
                }
            }

            return true;
        }
    
    private:
        // 使用 libcurl 发送 POST HTTP 请求
        bool sendRequestWithCurl(const std::string& url) 
        {
            CURL* curl = curl_easy_init();
            if (!curl) 
            {
                LOG_ERROR("初始化libcurl失败!");
                return false;
            }

            // 设置 URL - 分离URL和参数
            std::string baseUrl = url;
            std::string postFields = "";
            
            // 找到URL中的参数部分
            size_t pos = url.find('?');
            if (pos != std::string::npos) 
            {
                baseUrl = url.substr(0, pos);
                postFields = url.substr(pos + 1);
            }
            
            curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str()); // 设置基础URL
            curl_easy_setopt(curl, CURLOPT_POST, 1L); // 指定POST请求
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str()); // 设置POST数据
            curl_easy_setopt(curl, CURLOPT_CAINFO, _ca_path.c_str()); // 设置 CA 证书路径
            
            // 发送请求
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) 
            {
                LOG_ERROR("libcurl请求失败: {}", curl_easy_strerror(res));
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

    private:
        std::string _ca_path;
        std::unique_ptr<AlibabaCloud::CommonClient> _client;
    };
}