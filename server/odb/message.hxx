#pragma once

#include <string>
#include <cstddef>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>
//  odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time message.hxx

namespace IM
{
    #pragma db object table("message") 
    class Message
    {
    public:
        friend class odb::access;
    
        Message() = default;
    
        Message(const std::string& message_id,
                const std::string& session_id,
                const std::string& user_id,
                const unsigned char message_type,
                const boost::posix_time::ptime& create_time)
            : _message_id(message_id)
            , _session_id(session_id)
            , _user_id(user_id)
            , _message_type(message_type)
            , _create_time(create_time)
        {}

        unsigned int id() const { return _id; }
    
        std::string sessionId() const { return _session_id; }
        void sessionId(const std::string& session_id) { _session_id = session_id; }

        std::string userId() const { return _user_id; }
        void userId(const std::string& user_id) { _user_id = user_id; }

        std::string messageId() const { return _message_id; }
        void messageId(const std::string& message_id) { _message_id = message_id; }

        unsigned char messageType() const { return _message_type; }
        void messageType(const unsigned char message_type) { _message_type = message_type; }

        boost::posix_time::ptime createTime() const { return _create_time; }
        void createTime(const boost::posix_time::ptime& create_time) { _create_time = create_time; }

        std::string content() const { return _content ? *_content : ""; }
        void content(const std::string& content) { _content = content; }

        std::string fileId() const { return _file_id ? *_file_id : ""; }
        void fileId(const std::string& file_id) { _file_id = file_id; }

        std::string fileName() const { return _file_name ? *_file_name : ""; }
        void fileName(const std::string& file_name) { _file_name = file_name; }

        unsigned int fileSize() const { return _file_size ? *_file_size : 0; }
        void fileSize(const unsigned int file_size) { _file_size = file_size; }

    private:
        #pragma db id auto
        unsigned int _id; // 主键
    
        #pragma db type("varchar(64)") index unique
        std::string _message_id; // 消息id

        #pragma db type("varchar(64)")
        std::string _user_id; // 用户id

        #pragma db type("varchar(64)") index
        std::string _session_id; // 所属会话id
        unsigned char _message_type; // 消息类型

        #pragma db type("TIMESTAMP")
        boost::posix_time::ptime _create_time; // 消息时间
        odb::nullable<std::string> _content; // 文本消息内容

        #pragma db type("varchar(64)")
        odb::nullable<std::string> _file_id; // 文件消息id

        #pragma db type("varchar(128)")
        odb::nullable<std::string> _file_name; // 文件消息名称
        odb::nullable<unsigned int> _file_size; // 文件消息大小
    };
}
