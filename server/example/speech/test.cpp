#include "../../common/aip-cpp-sdk/speech.h"

void asr(aip::Speech& client)
{
    // 无可选参数调用接口
    std::string file_content;
    aip::get_file_content("16k.pcm", &file_content);
    Json::Value result = client.recognize(file_content, "pcm", 16000, aip::null);

    if (result["err_no"].asInt() != 0)
    {
        std::cout << result["err_msg"].asString() << std::endl;
        return;
    }

    std::cout << result["result"][0].asString() << std::endl;
}

int main()
{
    // 设置APPID/AK/SK
    std::string app_id = "117904044";
    std::string api_key = "h8lNpDJDAOibIrYHJmuiE6c6";
    std::string secret_key = "dPFk7GtT6ZZvupAhtOWfspBQZCsc55jF";

    aip::Speech client(app_id, api_key, secret_key);

    asr(client);
    
    return 0;
}


