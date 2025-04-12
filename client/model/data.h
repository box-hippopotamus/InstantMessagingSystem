#pragma once

#include "../common.h"

#include "base.qpb.h"
#include "gateway.qpb.h"
#include "user.qpb.h"
#include "friend.qpb.h"
#include "file.qpb.h"
#include "notify.qpb.h"
#include "speech_recognition.qpb.h"
#include "message_storage.qpb.h"
#include "message_transmit.qpb.h"

namespace IM::Model
{
    // 用户信息
    class UserInfo
    {
    public:
        QString userId;      // 用户id
        QString nickname;    // 用户名
        QString description; // 用户签名
        QString phone;       // 用户手机号
        QIcon avatar;        // 用户头像

        void load(const IM::Proto::UserInfo& userInfo)
        {
            userId = userInfo.userId();
            nickname = userInfo.nickname();
            phone = userInfo.phone();
            description = userInfo.description();
            avatar = userInfo.avatar().isEmpty()
                        ? QIcon(":/resource/image/defaultAvatar.png")
                        : makeIcon(userInfo.avatar());
        }
    };

    // 消息类型
    enum class MessageType
    {
        TEXT_TYPE,   // 文本消息
        IMAGE_TYPE,  // 图片消息
        FILE_TYPE,   // 文件消息
        SPEECH_TYPE, // 语音消息
    };

    // 消息信息
    class Message
    {
    public:
        Message() = default;

        static Message makeMessage(MessageType messageType,
                                   const QString& chatSessionId,
                                   const UserInfo& sender,
                                   const QByteArray& content,
                                   const QString& extraInfo)
        {
            switch(messageType)
            {
            case MessageType::TEXT_TYPE:
                return makeTextMessage(chatSessionId, sender, content);
            case MessageType::IMAGE_TYPE:
                return makeImageMessage(chatSessionId, sender, content);
            case MessageType::FILE_TYPE:
                return makeFileMessage(chatSessionId, sender, content, extraInfo);
            case MessageType::SPEECH_TYPE:
                return makeSpeechMessage(chatSessionId, sender, content);
            }

            return Message();
        }

        void load(const IM::Proto::MessageInfo& messageInfo)
        {
            messageId = messageInfo.messageId();
            chatSessionId = messageInfo.chatSessionId();
            time = formatTime(messageInfo.timestamp());
            sender.load(messageInfo.sender());

            // 设置消息类型
            auto& msg = messageInfo.message();
            switch (msg.messageType())
            {
            case IM::Proto::MessageTypeGadget::MessageType::STRING:
                content = msg.stringMessage().content().toUtf8();
                break;
            case IM::Proto::MessageTypeGadget::MessageType::IMAGE:
                if (msg.imageMessage().hasImageContent())
                    content = msg.imageMessage().imageContent();

                if (msg.imageMessage().hasFileId())
                    fileId = msg.imageMessage().fileId();
                break;
            case IM::Proto::MessageTypeGadget::MessageType::FILE:
                if (msg.fileMessage().hasFileContents())
                    content = msg.fileMessage().fileContents();

                if (msg.fileMessage().hasFileId())
                    fileId = msg.fileMessage().fileId();

                fileName = msg.fileMessage().fileName();
                break;
            case IM::Proto::MessageTypeGadget::MessageType::SPEECH:
                if (msg.speechMessage().hasFileContents())
                    content = msg.speechMessage().fileContents();

                if (msg.speechMessage().hasFileId())
                    fileId = msg.speechMessage().fileId();
                break;
            default:
                LOG() << "非法的消息类型! type=" << msg.messageType();
                return;
            }

            messageType = (MessageType)msg.messageType();
        }

    private:
        // 生成唯一 messageId
        static QString uuid()
        {
            return "M" + QUuid::createUuid().toString().sliced(25, 12);
        }

        static Message makeTextMessage(const QString& chatSessionId,
                                       const UserInfo& sender,
                                       const QByteArray& content)
        {
            Message message;
            message.messageId = uuid();
            message.chatSessionId = chatSessionId;
            message.sender = sender;
            message.time = formatTime(getTime());
            message.content = content;
            message.messageType = MessageType::TEXT_TYPE;

            message.fileId = "";
            message.fileName = "";
            return message;
        }

        static Message makeImageMessage(const QString& chatSessionId,
                                        const UserInfo& sender,
                                        const QByteArray& content)
        {
            Message message;
            message.messageId = uuid();
            message.chatSessionId = chatSessionId;
            message.sender = sender;
            message.time = formatTime(getTime());
            message.content = content;
            message.messageType = MessageType::IMAGE_TYPE;

            message.fileId = "";
            message.fileName = "";
            return message;
        }

        static Message makeFileMessage(const QString& chatSessionId,
                                       const UserInfo& sender,
                                       const QByteArray& content,
                                       const QString& fileName)
        {
            Message message;
            message.messageId = uuid();
            message.chatSessionId = chatSessionId;
            message.sender = sender;
            message.time = formatTime(getTime());
            message.content = content;
            message.messageType = MessageType::FILE_TYPE;

            message.fileId = "";
            message.fileName = fileName;
            return message;
        }

        static Message makeSpeechMessage(const QString& chatSessionId,
                                         const UserInfo& sender,
                                         const QByteArray& content)
        {
            Message message;
            message.messageId = uuid();
            message.chatSessionId = chatSessionId;
            message.sender = sender;
            message.time = formatTime(getTime());
            message.content = content;
            message.messageType = MessageType::SPEECH_TYPE;

            message.fileId = "";
            message.fileName = "";
            return message;
        }

    public:
        QString messageId;       // 消息id
        QString chatSessionId;   // 会话id
        QString time;            // 发送时间
        MessageType messageType; // 消息类型
        UserInfo sender;         // 发送者
        QByteArray content;      // 正文消息
        QString fileId;          // 文件id
        QString fileName;        // 文件名称
    };

    // 会话信息
    class ChatSessionInfo
    {
    public:
        QString chatSessionId;   // 会话id
        QString chatSessionName; // 会话名称
        Message lastMessage;     // 会话最后一条消息
        QIcon avatar;            // 会话头像
        QString userId;          // 单聊用户id

        void load(const IM::Proto::ChatSessionInfo& chatSessionInfo)
        {
            chatSessionId = chatSessionInfo.chatSessionId();
            chatSessionName = chatSessionInfo.chatSessionName();
            if (chatSessionInfo.hasSingleChatFriendId())
                userId = chatSessionInfo.singleChatFriendId();

            if (chatSessionInfo.hasPrevMessage())
                lastMessage.load(chatSessionInfo.prevMessage());

            // 设置头像
            if (chatSessionInfo.hasAvatar() && !chatSessionInfo.avatar().isEmpty())
            {
                avatar = makeIcon(chatSessionInfo.avatar());
            }
            else // 使用默认头像
            {
                avatar = QIcon((userId != "") // 群聊与单聊默认头像
                                    ? ":/resource/image/defaultAvatar.png"
                                    : ":/resource/image/groupAvatar.png");
            }
        }
    };
}
