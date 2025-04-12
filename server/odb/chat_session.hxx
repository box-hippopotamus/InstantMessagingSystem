#pragma once

#include <string>
#include <cstddef>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include "session_member.hxx"
//  odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time chat_session.hxx

namespace IM
{
    enum class ChatSessionType
    {
        SINGLE = 1,
        GROUP = 2
    };

    #pragma db object table("chat_session") 
    class ChatSession
    {
    public:
        friend class odb::access;
    
        ChatSession() = default;
    
        ChatSession(const std::string& session_id,
                    const std::string& session_name,
                    const ChatSessionType session_type)
            : _session_id(session_id)
            , _session_name(session_name)
            , _session_type(session_type)
        {}

        unsigned int id() const { return _id; }

        std::string sessionId() const { return _session_id; }
        void sessionId(const std::string& session_id) { _session_id = session_id; }

        std::string sessionName() const { return _session_name; }
        void sessionName(const std::string& session_name) { _session_name = session_name; }

        ChatSessionType sessionType() const { return _session_type; }
        void sessionType(ChatSessionType session_type) { _session_type = session_type; }

    private:
        #pragma db id auto
        unsigned int _id; // 主键
    
        #pragma db type("varchar(64)") index
        std::string _session_id; // 会话id

        #pragma db type("varchar(64)")
        std::string _session_name; // 会话名称

        #pragma db type("tinyint")
        ChatSessionType _session_type; // 会话类型
    };
    
    // 单人聊天会话查询:
    // select css.session_id, smb1.user_id 
    // from chat_session as css 
    // join session_member as smb1 on smb1.session_id == css.session_id and css.session_type == 1 -- 连接session_member表, 获取单聊会话成员
    // join session_member as smb2 on smb2.session_id == css.session_id and smb2.user_id != smb1.user_id -- 连接session_member表, 获取自己以外的会话成员
    // where cms.user_id == user_id;

    // query: css::session_type == 1 && smb1.user_id == user_id && smb2.user_id != smb1.user_id
    #pragma db view object(ChatSession = css)\
                    object(SessionMember = smb1 : css::_session_id == smb1::_session_id)\
                    object(SessionMember = smb2 : css::_session_id == smb2::_session_id)\
                    query((?))
    struct SingleChatSession
    {
        #pragma db column(css::_session_id)
        std::string session_id;
        
        #pragma db column(smb2::_user_id)
        std::string friend_id;
    };

    // query: css::session_type == 2 && smb.user_id == user_id
    #pragma db view object(ChatSession = css)\
                    object(SessionMember = smb : css::_session_id == smb::_session_id)\
                    query((?))
    struct GroupChatSession
    {
        #pragma db column(css::_session_id)
        std::string session_id;
        
        #pragma db column(css::_session_name)
        std::string session_name;
    };
}
