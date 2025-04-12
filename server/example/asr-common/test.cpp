#include "../../common/asr.hpp"
#include <gflags/gflags.h>

DEFINE_string(app_id, "117904044", "app_id");
DEFINE_string(api_key, "h8lNpDJDAOibIrYHJmuiE6c6", "api_key");
DEFINE_string(secret_key, "dPFk7GtT6ZZvupAhtOWfspBQZCsc55jF", "secret_key");

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

using namespace IM;

int main(int argc, char** argv )
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    std::string file_content;
    aip::get_file_content("16k.pcm", &file_content);

    AsrClient client(FLAGS_app_id, FLAGS_api_key, FLAGS_secret_key);
    std::string err;
    std::cout << client.recognize(file_content, err) << std::endl;

    return 0;
}