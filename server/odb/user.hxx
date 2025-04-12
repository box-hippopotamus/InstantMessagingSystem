#pragma once

#include <string>
#include <cstddef>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
//  odb -d mysql --std c++11 --generaZte-query --generate-schema --profile boost/date-time user.hxx

namespace IM
{
    #pragma db object table("user") 
    class User
    {
    public:
        friend class odb::access;
    
        User() = default;
    
        // 通过用户名新增用户
        User(const std::string& user_id, const std::string& nickname, const std::string& password)
            : _user_id(user_id)
            , _nickname(nickname)
            , _password(password)
        {}
    
        // 通过手机号新增用户
        User(const std::string& user_id, const std::string& phone)
            : _user_id(user_id)
            , _phone(phone)
            , _nickname(user_id)
        {}
    
        unsigned int id() const { return _id; }
    
        std::string userId() const { return _user_id; }
        void userId(const std::string& user_id) { _user_id = user_id; }
    
        std::string nickname() const { return _nickname ? *_nickname : ""; }
        void nickname(const std::string& nickname) { _nickname = nickname; }
    
        std::string description() const { return _description ? *_description : ""; }
        void description(const std::string& description) { _description = description; }
    
        std::string password() const { return _password ? *_password : ""; }
        void password(const std::string& password) { _password = password; }
    
        std::string phone() const { return _phone ? *_phone : ""; }
        void phone(const std::string& phone) { _phone = phone; }
    
        std::string avatarId() const { return _avatar_id ? *_avatar_id : ""; }
        void avatarId(const std::string& avatar_id) { _avatar_id = avatar_id; }
    
    private:
        #pragma db id auto
        unsigned int _id; // 主键
    
        #pragma db type("varchar(64)") index unique
        std::string _user_id; // 用户id
    
        #pragma db type("varchar(64)") index unique
        odb::nullable<std::string> _nickname; // 用户名
    
        odb::nullable<std::string> _description; // 用户签名
    
        #pragma db type("varchar(64)")
        odb::nullable<std::string> _password; // 用户密码
    
        #pragma db type("varchar(64)") index unique
        odb::nullable<std::string> _phone; // 用户手机号
    
        #pragma db type("varchar(64)")
        odb::nullable<std::string> _avatar_id; // 头像id    
    };
}
