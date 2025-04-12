#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

// 日志等级:
// namespace level {
//     enum level_enum : int {
//         trace = SPDLOG_LEVEL_TRACE,
//         debug = SPDLOG_LEVEL_DEBUG,
//         info = SPDLOG_LEVEL_INFO,
//         warn = SPDLOG_LEVEL_WARN,
//         err = SPDLOG_LEVEL_ERROR,
//         critical = SPDLOG_LEVEL_CRITICAL,
//         off = SPDLOG_LEVEL_OFF,
//         n_levels
//     };
// }

// 日志记录器类：class logger 完成同步日志输出

// 异步日志记录类：class async_logger 完成异步日志输出

// 日志工厂类

// 日志落地类 class SPDLOG_API sink 决定日志的输出位置（标准输出，标志错误，文件）
// 可继承，重写虚函数，自定义输出

// logger->trace("This is a trace message");
// logger->debug("This is a debug message");
// logger->info("This is an info message");
// logger->warn("This is a warning message");
// logger->error("This is an error message");
// logger->critical("This is a critical message");

int main(int argc, char* argv[])
{
    // 设置全局刷新策略
    spdlog::flush_every(std::chrono::seconds(1)); // 每秒刷新
    spdlog::flush_on(spdlog::level::level_enum::debug); // 遇到debug以上的日志，立即刷新

    // 全局日志输出等级(每个日志后续也可以自行设定)
    spdlog::set_level(spdlog::level::level_enum::debug);

    // 初始化异步日志线程配置
    //                     节点数量 线程数量
    spdlog::init_thread_pool(1024, 1);

    // 通过工厂创建日志器 (同步)
    //                                     日志器名称
    // auto logger = spdlog::stdout_color_mt("sync-logger"); // 标准输出
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("async-logger"); // 异步标准输出
    // auto logger = spdlog::basic_logger_mt("file-logger", "file.log"); // 文件输出

    // 设置日志器刷新策略
    // looger->flush_on(spdlog::level::level_enum::debug);
    // looger->set_level(spdlog::level::level_enum::debug);
    // 之前已经全局设置了

    // 自定义日志格式
    logger->set_pattern("[%n]:%Y-%m-%d %H:%M:%S [%t] [%-7l] %v");
    // %t - 线程 ID（Thread ID）
    // %n - 日志器名称（Logger name）
    // %l - 日志级别名称（Level name），如 INFO, DEBUG, ERROR 等
    // %v - 日志内容（message）
    // %Y - 年（Year）
    // %m - 月（Month）
    // %d - 日（Day）
    // %H - 小时（24-hour format）
    // %M - 分钟（Minute）
    // %S - 秒（Second）

    int num = 20;
    std::string str = "world";
    logger->debug("hello! {}! {}", str, num);
    //               使用 {} 进行占位，不考虑类型

    return 0;
}