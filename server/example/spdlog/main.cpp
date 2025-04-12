#include <iostream>
#include "logger.hpp"
#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    LOG_DEBUG("你好 {}", "world");
    LOG_INFO("你好 {}", "world");
    LOG_WARN("你好 {}", "world");
    LOG_ERROR("你好 {}", "world");
    LOG_FATAL("你好 {}", "world");

    return 0;
}