#pragma once

#include <string>
#include <cstddef>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
//  odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time session_member.hxx

namespace IM
{
    #pragma db object table("session_member") 
    class SessionMember
    {
    public:
        friend class odb::access;
    
        SessionMember() = default;
    
        SessionMember(const std::string& session_id, const std::string& user_id)
            : _session_id(session_id)
            , _user_id(user_id)
        {}

        unsigned int id() const { return _id; }
    
        std::string sessionId() const { return _session_id; }
        void sessionId(const std::string& session_id) { _session_id = session_id; }

        std::string userId() const { return _user_id; }
        void userId(const std::string& user_id) { _user_id = user_id; }

    private:
        #pragma db id auto
        unsigned int _id; // 主键
    
        #pragma db type("varchar(64)") index 
        std::string _session_id; // 会话id

        #pragma db type("varchar(64)")
        std::string _user_id; // 用户id
    };
}
