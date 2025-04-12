#pragma once

#include "../third/include/aip-cpp-sdk/speech.h"
#include "logger.hpp"

namespace IM
{
    class AsrClient
    {
    public:
        using ptr = std::shared_ptr<AsrClient>;

        AsrClient(const std::string& app_id, 
                    const std::string& api_key, 
                    const std::string& secret_key)
                : _client(app_id, api_key, secret_key)
        {}

        std::string recognize(const std::string& speech_data, std::string& error_msg)
        {
            Json::Value result = _client.recognize(speech_data, "pcm", 16000, aip::null);
        
            if (result["err_no"].asInt() != 0)
            {
                LOG_ERROR("语音识别失败: {}", result["err_msg"].asString());
                error_msg = result["err_msg"].asString();
                return "";
            }
        
            return result["result"][0].asString();
        }


    private:
        aip::Speech _client;
    };
}