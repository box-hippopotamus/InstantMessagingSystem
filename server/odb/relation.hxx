#pragma once

#include <string>
#include <cstddef>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
//  odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time relation.hxx

namespace IM
{
    #pragma db object table("relation") 
    class Relation
    {
    public:
        friend class odb::access;
    
        Relation() = default;
    
        Relation(const std::string& user_id,
                 const std::string& peer_id)
            : _user_id(user_id)
            , _peer_id(peer_id)
        {}

        unsigned int id() const { return _id; }

        std::string userId() const { return _user_id; }
        void userId(const std::string& user_id) { _user_id = user_id; }

        std::string peerId() const { return _peer_id; }
        void peerId(const std::string& peer_id) { _peer_id = peer_id; }

    private:
        #pragma db id auto
        unsigned int _id; // 主键
    
        #pragma db type("varchar(64)") index
        std::string _user_id; // 用户id

        #pragma db type("varchar(64)")
        std::string _peer_id; // 好友id
    };
}
