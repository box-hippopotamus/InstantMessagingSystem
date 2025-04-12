#pragma once

#include <iostream>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

namespace IM
{
    // mode: true发布模式，false调试模式
    //       日志输出到文件   输出到标准流
    std::shared_ptr<spdlog::logger> g_default_logger; // 全局日志器
    void init_logger(bool mode, const std::string& file, int32_t level)
    {
        if (!mode)
        {
            g_default_logger = spdlog::stdout_color_mt("default-logger");
            g_default_logger->set_level(spdlog::level::level_enum::trace);
            g_default_logger->flush_on(spdlog::level::level_enum::trace);
        }
        else
        {
            g_default_logger = spdlog::basic_logger_mt("default-logger", file);
            g_default_logger->set_level((spdlog::level::level_enum)level);
            g_default_logger->flush_on((spdlog::level::level_enum)level);
        }
        g_default_logger->set_pattern("[%n]:%Y-%m-%d %H:%M:%S [%t] [%-7l] %v");
    }

    #define LOG_TRACE(format, ...) g_default_logger->trace(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define LOG_DEBUG(format, ...) g_default_logger->debug(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define LOG_INFO(format, ...) g_default_logger->info(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define LOG_WARN(format, ...) g_default_logger->warn(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define LOG_ERROR(format, ...) g_default_logger->error(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define LOG_FATAL(format, ...) g_default_logger->critical(std::string("[{}:{}]") + format, __FILE__, __LINE__, ##__VA_ARGS__)
}