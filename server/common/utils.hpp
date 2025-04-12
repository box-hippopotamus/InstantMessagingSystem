#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <random>
#include <atomic>
#include <iomanip> // setw

#include "logger.hpp"

namespace IM
{
    class UUID
    {
    public:
        static std::string uuid()
        {
            // uuid: 12字节随机数 + 4字节自增序号
            std::stringstream ss;
    
            // 机器随机数，通过硬件实现，随机性强，但是慢
            std::random_device rd; 
    
            // 以机器随机数为种子，构造伪随机数对象(此处不使用时间，防止短时间内多次生成随机数一样)
            std::mt19937 generator(rd());
    
            // 限定随机数范围，指定 0-255 也就是一次生成一个字节
            std::uniform_int_distribution<int> distribution(0, 255); 
    
            for (int i = 0; i < 6; i++)
            {
                //    限制两位宽度     不足的位置填充 0     转为十六进制   限定范围    生成随机数 
                ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
            }
    
            static std::atomic<size_t> seq(1); // 全局自增变量
            size_t tmp = seq.fetch_add(1);
            ss << std::setw(4) << std::setfill('0') << std::hex << tmp;

            return ss.str();
        }
        
        static std::string vcode()
        {
            std::stringstream ss;
            std::random_device rd; 
            std::mt19937 generator(rd());
            std::uniform_int_distribution<int> distribution(1000, 9999); 
    
            return std::to_string(distribution(generator));
        }
    };

    class FileOp
    {
    public:
        static bool readFile(const std::string& filename, std::string& body)
        {
            std::ifstream ifs(filename, std::ios::binary | std::ios::in);
            if (!ifs.is_open())
            {
                LOG_ERROR("打开 {} 文件失败!", filename);
                ifs.close();
                return false;
            }

            ifs.seekg(0, std::ios::end); // 跳转到文件末尾
            size_t flen = ifs.tellg(); // 获取当前偏移量->文件大小
            ifs.seekg(0, std::ios::beg); // 跳转到文件末尾

            body.resize(flen);
            ifs.read(&body[0], flen);
            if (!ifs.good())
            {
                LOG_ERROR("读取 {} 文件失败!", filename);
                ifs.close();
                return false;
            }

            ifs.close();
            return true;
        }

        static bool writeFile(const std::string& filename, const std::string& body)
        {
            std::ofstream ofs(filename, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!ofs.is_open())
            {
                LOG_ERROR("打开 {} 文件失败!", filename);
                ofs.close();
                return false;
            }

            ofs.write(body.c_str(), body.size());
            if (!ofs.good())
            {
                LOG_ERROR("写入 {} 文件失败!", filename);
                ofs.close();
                return false;
            }

            ofs.close();
            return true;
        }
    };
}
