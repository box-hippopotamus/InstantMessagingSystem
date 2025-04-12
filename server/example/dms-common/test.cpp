#include "../../common/dms.hpp"
#include <gflags/gflags.h>

DEFINE_string(access_key_id, "LTAI5t5uWQw6qW1fdM6EEYPk", "access_id");
DEFINE_string(access_key_secret, "XwUM8TRs7huj9VoPjhh6Lg3IPGsW4N", "access_secret");
DEFINE_string(ca_path, "./", "CA证书路径，必须以/结尾");

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");


int main( int argc, char** argv )
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    IM::DMSClient client(FLAGS_access_key_id, FLAGS_access_key_secret, FLAGS_ca_path);
    client.send("15633890522", "6688");

    return 0;
}