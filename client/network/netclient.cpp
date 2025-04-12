#include "netclient.h"
#include "../model/datacenter.h"

namespace IM::Network
{
    using namespace Model;

    NetClient::NetClient(Model::DataCenter *_dataCenter)
        : dataCenter(_dataCenter)
    {}

    void NetClient::ping()
    {
        QNetworkRequest httpReq;
        httpReq.setUrl(QUrl(HTTP_URL + "/ping"));

        QNetworkReply* httpResp = httpClient.get(httpReq);

        // QNetworkReply::finished 收到报文响应的信号
        connect(httpResp, &QNetworkReply::finished, this, [=]() {
            if (httpResp->error() != QNetworkReply::NoError) // 请求失败
            {
                LOG() << "HTTP 请求失败! " << httpResp->errorString();
                httpResp->deleteLater();
                return;
            }

            QByteArray body = httpResp->readAll();
            LOG() << "响应内容: " << body;
            httpResp->deleteLater();
        });
    }

    void NetClient::initWebsocket()
    {
        connect(&websocketClient, &QWebSocket::connected, this, [this]() {
            LOG() << "websocket 连接成功!";
            sendAuth(); // websocket 连接成功之后, 发送身份认证消息
        });

        connect(&websocketClient, &QWebSocket::disconnected, this, [this]() {
            LOG() << "websocket 连接断开!";
        });

        connect(&websocketClient, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
            LOG() << "websocket 连接出错!" << error;
        });

        connect(&websocketClient, &QWebSocket::textMessageReceived, this, [this](const QString& message) {
            LOG() << "websocket 收到文本消息!" << message;
        });

        connect(&websocketClient, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray& byteArray) {
            LOG() << "websocket 收到二进制消息!" << byteArray.length();
            Proto::NotifyMessage notifyMessage;
            notifyMessage.deserialize(&serializer, byteArray);
            handleWsResponse(notifyMessage);
        });

        websocketClient.open(WEBSOCKET_URL);
    }

    void NetClient::closeWebsocket()
    {
        websocketClient.close();
        LOG() << "close websocket";
    }

    void NetClient::handleWsResponse(const Proto::NotifyMessage &notifyMessage)
    {
        LOG() << "收到ws消息";

        switch(notifyMessage.notifyType())
        {
            case Proto::NotifyTypeGadget::NotifyType::CHAT_MESSAGE_NOTIFY:
            {
                        LOG() << "收到ws消息";
                //  pb 中的 MessageInfo 转成客户端 Message
                Message message;
                message.load(notifyMessage.newMessageInfo().messageInfo());
                handleWsMessage(message);
                break;
            }
            case Proto::NotifyTypeGadget::NotifyType::CHAT_SESSION_CREATE_NOTIFY:
            {
                        LOG() << "收到ws消息";
                // 创建新的会话通知
                ChatSessionInfo chatSessionInfo;
                chatSessionInfo.load(notifyMessage.newChatSessionInfo().chatSessionInfo());
                handleWsSessionCreate(chatSessionInfo);
                break;
            }
            case Proto::NotifyTypeGadget::NotifyType::FRIEND_ADD_APPLY_NOTIFY:
            {
                        LOG() << "收到ws消息";
                // 添加好友申请通知
                UserInfo userInfo;
                userInfo.load(notifyMessage.friendAddApply().userInfo());
                handleWsAddFriendApply(userInfo);
                break;
            }
            case Proto::NotifyTypeGadget::NotifyType::FRIEND_ADD_PROCESS_NOTIFY:
            {
                        LOG() << "收到ws消息";
                // 添加好友申请的处理结果通知
                UserInfo userInfo;
                userInfo.load(notifyMessage.friendProcessResult().userInfo());
                bool agree = notifyMessage.friendProcessResult().agree();
                handleWsAddFriendProcess(userInfo, agree);
                break;
            }
            case Proto::NotifyTypeGadget::NotifyType::FRIEND_REMOVE_NOTIFY:
            {
                        LOG() << "收到ws消息";
                // 删除好友通知
                const QString& userId = notifyMessage.friendRemove().userId();
                handleWsRemoveFriend(userId);
                break;
            }
            default:
                LOG() << "不存在的消息类型!";
        }
    }

    void NetClient::handleWsMessage(const Model::Message &message)
    {
        QList<Message>* messageList = dataCenter->getRecentMessageList(message.chatSessionId);
        if (messageList == nullptr)
        {
            // 通过网络先加载整个消息列表
            connect(dataCenter, &DataCenter::getRecentMessageListDoneNoUI, this, &NetClient::receiveMessage, Qt::UniqueConnection);
            dataCenter->getRecentMessageListAsync(message.chatSessionId, false);
        }
        else
        {
            // 消息尾插到消息列表中
            messageList->push_back(message);
            receiveMessage(message.chatSessionId);
        }
    }

    void NetClient::handleWsRemoveFriend(const QString &userId)
    {
        // 删除 DataCenter 好友列表的数据
        dataCenter->removeFriend(userId);
        // 通知界面变化. 更新 好友列表 / 会话列表
        emit dataCenter->deleteFriendDone();
    }

    void NetClient::handleWsAddFriendApply(const Model::UserInfo &userInfo)
    {
        // 把数据添加到 DataCenter 好友申请列表
        QList<UserInfo>* applyList = dataCenter->getApplyList();
        if (applyList == nullptr)
        {
            LOG() << "客户端没有加载到好友申请列表!";
            return;
        }

        applyList->push_front(userInfo); // 把新的元素放到列表前面
        emit dataCenter->receiveFriendApplyDone(); // 通知界面更新
    }

    void NetClient::handleWsAddFriendProcess(const Model::UserInfo &userInfo, bool agree)
    {
        if (!agree) // 拒绝好友申请
        {
            emit dataCenter->receiveFriendProcessDone(userInfo.nickname, agree);
            return;
        }

        // 同意好友申请
        QList<UserInfo>* friendList = dataCenter->getFriendList();
        if (friendList == nullptr)
        {
            LOG() << "客户端没有加载好友列表";
            return;
        }

        friendList->push_front(userInfo);
        emit dataCenter->receiveFriendProcessDone(userInfo.nickname, agree);
    }

    void NetClient::handleWsSessionCreate(const Model::ChatSessionInfo& chatSessionInfo)
    {
        //  ChatSessionInfo 添加到会话列表
        QList<ChatSessionInfo>* chatSessionList = dataCenter->getChatSessionList();
        if (chatSessionList == nullptr)
        {
            LOG() << "客户端没有加载会话列表";
            return;
        }

        chatSessionList->push_front(chatSessionInfo); // 新的元素添加到列表头部.
        emit dataCenter->receiveSessionCreateDone(); // 发送一个信号, 通知界面更新
    }

    void NetClient::sendAuth()
    {
        Proto::ClientAuthenticationReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(dataCenter->getLoginSessionId());
        QByteArray body = req.serialize(&serializer);
        websocketClient.sendBinaryMessage(body);
        LOG() << "[WS身份认证] requestId=" << req.requestId()
              << ", loginSessionId=" << req.sessionId();
    }

    QString NetClient::makeRequestId()
    {
        return "R" + QUuid::createUuid().toString().sliced(25, 12);
    }

    // 发送 HTTP 请求
    QNetworkReply *NetClient::sendHttpRequest(const QString &apiPath, const QByteArray &body)
    {
        QNetworkRequest httpReq;
        httpReq.setUrl(QUrl(HTTP_URL + apiPath));
        httpReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-protobuf");

        QNetworkReply* httpResp = httpClient.post(httpReq, body);
        return httpResp;
    }

    void NetClient::getMyself(const QString &loginSessionId)
    {
        // 构造 body
        Proto::GetUserInfoReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        QByteArray body = req.serialize(&serializer);
        LOG() << "[获取个人信息] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId;

        // 发送请求
        QNetworkReply* httpResp = sendHttpRequest("/service/user/get_user_info", body);

        // 处理响应
        connect(httpResp, &QNetworkReply::finished, this, [=]() {
            // 处理响应对象
            bool ok = false;
            QString reason;
            auto resp = handleHttpResponse<Proto::GetUserInfoRsp>(httpResp, &ok, &reason);

            if (!ok)
            {
                LOG() << "[获取个人信息] 出错! requestId=" << req.requestId() << "reason=" << reason;
                return;
            }

            dataCenter->resetMyself(resp); // 响应数据保存到 DataCenter
            emit dataCenter->getMyselfDone(); // 通知响应处理完毕
            LOG() << "[获取个人信息] 处理响应 requestId=" << req.requestId();
        });
    }

    void NetClient::getFriendList(const QString& loginSessionId)
    {
        // 构造 body
        Proto::GetFriendListReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        QByteArray body = req.serialize(&serializer);
        LOG() << "[获取好友列表] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId;

        // 发送 HTTP 请求
        QNetworkReply* httpResp = sendHttpRequest("/service/friend/get_friend_list", body);

        // 处理响应
        connect(httpResp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto friendListResp = handleHttpResponse<Proto::GetFriendListRsp>(httpResp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[获取好友列表] 失败! requestId=" << req.requestId() << ", reason=" << reason;
                return;
            }

            dataCenter->resetFriendList(friendListResp);
            emit dataCenter->getFriendListDone();
            LOG() << "[获取好友列表] 处理响应 requestId=" << req.requestId();
        });
    }

    void NetClient::getChatSessionList(const QString &loginSessionId)
    {
        // 构造 body
        Proto::GetChatSessionListReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        QByteArray body = req.serialize(&serializer);
        LOG() << "[获取会话列表] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/get_chat_session_list", body);

        // 响应处理
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GetChatSessionListRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[获取会话列表] 失败! reason=" << reason;
                return;
            }

            dataCenter->resetChatSessionList(pbResp);
            emit dataCenter->getChatSessionListDone();
            LOG() << "[获取会话列表] 处理响应完毕! requestId=" << pbResp->requestId();
        });
    }

    void NetClient::getApplyList(const QString &loginSessionId)
    {
        // 构造 body
        Proto::GetPendingFriendEventListReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        QByteArray body = req.serialize(&serializer);
        LOG() << "[获取好友申请列表] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId;

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/get_pending_friend_events", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GetPendingFriendEventListRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[获取好友申请列表] 失败! reason=" << reason;
                return;
            }

            dataCenter->resetApplyList(pbResp);
            emit dataCenter->getApplyListDone();
            LOG() << "[获取好友申请列表] 处理响应完成! requestId=" << req.requestId();
        });
    }

    void NetClient::groupAddMember(const QString &loginSessionId, const QString &chatSessionId, const QList<QString> &userIdList)
    {
        // 构造 body
        Proto::GroupAddMemberReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        req.setChatSessionId(chatSessionId);
        req.setMemberIdList(userIdList);

        QByteArray body = req.serialize(&serializer);
        LOG() << "[添加群聊成员] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId;

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/group_add_member", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GroupAddMemberRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[添加群聊成员] 失败! reason=" << reason;
                return;
            }

            QList<UserInfo> userList;
            for (const auto& user_info : pbResp->memberInfoList())
            {
                UserInfo info;
                info.load(user_info);
                userList.push_back(info);
            }

            dataCenter->addChatSessionMember(chatSessionId, userList);
            emit dataCenter->groupAddMemberDone();
            LOG() << "[添加群聊成员] 处理响应完成! requestId=" << req.requestId();
        });
    }

    void NetClient::groupModifyName(const QString &loginSessionId, const QString &chatSessionId, const QString &groupName)
    {
        // 构造 body
        Proto::GroupModifyNameReq req;
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        req.setChatSessionId(chatSessionId);
        req.setChatSessionName(groupName);

        QByteArray body = req.serialize(&serializer);
        LOG() << "[修改群聊名称] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId
              << ", loginSessionId=" << loginSessionId
              << ", chatSessionId=" << chatSessionId
              << ", groupName=" << groupName;

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/group_modify_name", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GroupModifyNameRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[修改群聊名称] 失败! reason=" << reason;
                return;
            }

            dataCenter->modifyGroupName(chatSessionId, groupName);
            emit dataCenter->groupModifyNameDone(chatSessionId, groupName);
            LOG() << "[修改群聊名称] 处理响应完成! requestId=" << req.requestId();
        });
    }

    void NetClient::leaveGroup(const QString &loginSessionId, const QString &chatSessionId)
    {
        Proto::MemberLeaveGroupReq req;
        req.setChatSessionId(chatSessionId);
        req.setRequestId(makeRequestId());
        req.setSessionId(loginSessionId);
        QByteArray body = req.serialize(&serializer);
        LOG() << "[退出群聊] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId
              << ", chatSessionId=" << chatSessionId;

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/member_leave_group", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::MemberLeaveGroupRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[退出群聊] 失败! reason=" << reason;
                return;
            }

            dataCenter->deleteChatSession(chatSessionId);
            emit dataCenter->leaveGroupDone(chatSessionId);
            LOG() << "[退出群聊] 处理响应完成! requestId=" << req.requestId();
        });
    }

    void NetClient::getRecentMessageList(const QString &loginSessionId, const QString &chatSessionId, bool updateUI)
    {
        // 构造请求 body
        Proto::GetRecentMsgReq req;
        req.setRequestId(makeRequestId());
        req.setChatSessionId(chatSessionId);
        req.setMsgCount(50);	// 此处固定获取最近 50 条记录
        req.setSessionId(loginSessionId);
        QByteArray body = req.serialize(&serializer);
        LOG() << "[获取最近消息] 发送请求 requestId=" << req.requestId()
              << ", loginSessionId=" << loginSessionId
              << ", chatSessionId=" << chatSessionId;

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/message_storage/get_recent", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GetRecentMsgRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[获取最近消息] 失败! reason=" << reason;
                return;
            }

            dataCenter->resetRecentMessageList(chatSessionId, pbResp);
            if (updateUI)
                emit dataCenter->getRecentMessageListDone(chatSessionId);
            else
                emit dataCenter->getRecentMessageListDoneNoUI(chatSessionId);
        });
    }

    void NetClient::sendMessage(const QString &loginSessionId,
                                const QString &chatSessionId,
                                MessageType messageType,
                                const QByteArray &content,
                                const QString& extraInfo) // 文件名
    {
        // 构造 body
        Proto::NewMessageReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setChatSessionId(chatSessionId);

        // 构造 MessageContent
        Proto::MessageContent messageContent;

        switch(messageType)
        {
            case MessageType::TEXT_TYPE:
            {
                Proto::StringMessageInfo stringMessageInfo;
                stringMessageInfo.setContent(content);
                messageContent.setStringMessage(stringMessageInfo);
                messageContent.setMessageType(Proto::MessageTypeGadget::MessageType::STRING);
                break;
            }
            case MessageType::IMAGE_TYPE:
            {
                Proto::ImageMessageInfo imageMessageInfo;
                imageMessageInfo.setFileId("");
                imageMessageInfo.setImageContent(content);
                messageContent.setImageMessage(imageMessageInfo);
                messageContent.setMessageType(Proto::MessageTypeGadget::MessageType::IMAGE);
                break;
            }
            case MessageType::FILE_TYPE:
            {
                Proto::FileMessageInfo fileMessageInfo;
                fileMessageInfo.setFileId("");
                fileMessageInfo.setFileSize(content.size());
                fileMessageInfo.setFileName(extraInfo);
                fileMessageInfo.setFileContents(content);
                messageContent.setFileMessage(fileMessageInfo);
                messageContent.setMessageType(Proto::MessageTypeGadget::MessageType::FILE);
                break;
            }
            case MessageType::SPEECH_TYPE:
            {
                Proto::SpeechMessageInfo speechMessageInfo;
                speechMessageInfo.setFileId("");
                speechMessageInfo.setFileContents(content);
                messageContent.setSpeechMessage(speechMessageInfo);
                messageContent.setMessageType(Proto::MessageTypeGadget::MessageType::SPEECH);
                break;
            }
            default:
                LOG() << "错误的消息类型! messageType=" << (int)messageType;
        }

        pbReq.setMessage(messageContent);

        // 序列化
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[发送消息] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", chatSessionId=" << pbReq.chatSessionId()
              << ", messageType=" << pbReq.message().messageType();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/message_transmit/new_message", body);

        // 处理 HTTP 响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::NewMessageRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[发送消息] 处理出错! reason=" << reason;
                return;
            }

            emit dataCenter->sendMessageDone(messageType, content, extraInfo);
            LOG() << "[发送消息] 响应处理完毕! requestId=" << pbResp->requestId();
        });
    }

    void NetClient::receiveMessage(const QString &chatSessionId)
    {
        if (chatSessionId == dataCenter->getCurrentChatSessionId())
        {
            const Message& lastMessage = dataCenter->getRecentMessageList(chatSessionId)->back();
            emit dataCenter->receiveMessageDone(lastMessage);
        }
        else
        {
            dataCenter->addUnread(chatSessionId);
        }

        emit dataCenter->updateLastMessage(chatSessionId);
    }

    void NetClient::changeNickname(const QString &loginSessionId, const QString &nickname)
    {
        // 构造 body
        Proto::SetUserNicknameReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setNickname(nickname);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[修改用户昵称] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", nickname=" << pbReq.nickname();

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/set_nickname", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::SetUserNicknameRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[修改用户昵称] 出错! reason=" << reason;
                return;
            }

            dataCenter->resetNickname(nickname);
            emit dataCenter->changeNicknameDone();
            LOG() << "[修改用户昵称] 处理响应完毕! requestId=" << pbResp->requestId();
        });
    }

    void NetClient::changeDescription(const QString &loginSessionId, const QString &desc)
    {
        // 构造请求 body
        Proto::SetUserDescriptionReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setDescription(desc);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[修改签名] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessisonId=" << pbReq.sessionId()
              << ", desc=" << pbReq.description();

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/set_description", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::SetUserDescriptionRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[修改签名] 响应失败! reason=" << reason;
                return;
            }

            dataCenter->resetDescription(desc);
            emit dataCenter->changeDescriptionDone();
            LOG() << "[修改签名] 响应完成! requestId=" << pbResp->requestId();
        });
    }

    void NetClient::getVerifyCode(const QString &phone)
    {
        // 构造请求 body
        Proto::PhoneVerifyCodeReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setPhoneNumber(phone);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[获取手机验证码] 发送请求 requestId=" << pbReq.requestId()
              << ", phone=" << phone;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/get_phone_verify_code", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::PhoneVerifyCodeRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[获取手机验证码] 失败! reason=" << reason;
                return;
            }

            dataCenter->resetVerifyCodeId(pbResp->verifyCodeId());
            emit dataCenter->getVerifyCodeDone();
            LOG() << "[获取手机验证码] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::changePhone(const QString &loginSessionId, const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
    {
        // 构造请求 body
        Proto::SetUserPhoneNumberReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setPhoneNumber(phone);
        pbReq.setPhoneVerifyCodeId(verifyCodeId);
        pbReq.setPhoneVerifyCode(verifyCode);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[修改手机号] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", phone=" << pbReq.phoneNumber()
              << ", verifyCodeId=" << pbReq.phoneVerifyCodeId()
              << ", verifyCode=" << pbReq.phoneVerifyCode();

        // 发送 http 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/set_phone", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::SetUserPhoneNumberRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[修改手机号] 响应失败! reason=" << reason;
                return;
            }

            dataCenter->resetPhone(phone);
            emit dataCenter->changePhoneDone();
            LOG() << "[修改手机号] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::changeAvatar(const QString &loginSessionId, const QByteArray &avatar)
    {
        // 构造请求 body
        Proto::SetUserAvatarReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setAvatar(avatar);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[修改头像] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/set_avatar", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::SetUserAvatarRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[修改头像] 响应出错! reason=" << reason;
                return;
            }

            dataCenter->resetAvatar(avatar);
            emit dataCenter->changeAvatarDone();
            LOG() << "[修改头像] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::deleteFriend(const QString &loginSessionId, const QString &userId)
    {
        // 构造请求 body
        Proto::FriendRemoveReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setPeerId(userId);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[删除好友] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", peerId=" << pbReq.peerId();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/remove_friend", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::FriendRemoveRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[删除好友] 响应失败! reason=" << reason;
                return;
            }

            dataCenter->removeFriend(userId);
            emit dataCenter->deleteFriendDone();
            LOG() << "[删除好友] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::addFriendApply(const QString &loginSessionId, const QString &userId)
    {
        // 构造请求 body
        Proto::FriendAddReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setRespondentId(userId);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[添加好友申请] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", userId=" << userId;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/add_friend_apply", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::FriendAddRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[添加好友申请] 响应失败! reason=" << reason;
                return;
            }

            emit dataCenter->addFriendApplyDone();
            LOG() << "[添加好友申请] 响应完毕 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::acceptFriendApply(const QString &loginSessionId, const QString &userId)
    {
        // 构造请求 body
        Proto::FriendAddProcessReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setAgree(true);
        pbReq.setApplyUserId(userId);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[同意好友申请] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", userId=" << pbReq.applyUserId();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/add_friend_process", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::FriendAddRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[同意好友申请] 处理失败! reason=" << reason;
                return;
            }

            UserInfo applyUser = dataCenter->removeFromApplyList(userId);
            QList<UserInfo>* friendList = dataCenter->getFriendList();
            friendList->push_front(applyUser);

            emit dataCenter->acceptFriendApplyDone();
            LOG() << "[同意好友申请] 响应完成! requestId=" << pbResp->requestId();
        });
    }

    void NetClient::rejectFriendApply(const QString &loginSessionId, const QString &userId)
    {
        // 构造请求 body
        Proto::FriendAddProcessReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setAgree(false);
        pbReq.setApplyUserId(userId);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[拒绝好友申请] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", userId=" << pbReq.applyUserId();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/add_friend_process", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::FriendAddRsp>(resp, &ok, &reason);
            if (!ok) {
                LOG() << "[拒绝好友申请] 处理失败! reason=" << reason;
                return;
            }

            dataCenter->removeFromApplyList(userId);
            emit dataCenter->rejectFriendApplyDone();
            LOG() << "[拒绝好友申请] 响应完成! requestId=" << pbResp->requestId();
        });
    }

    void NetClient::createGroupChatSession(const QString &loginSessionId, const QList<QString> &userIdList)
    {
        // 构造请求 body
        Proto::ChatSessionCreateReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setChatSessionName("新的群聊");
        pbReq.setMemberIdList(userIdList);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[创建群聊会话] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << loginSessionId
              << ", userIdList=" << userIdList;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/create_chat_session", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::ChatSessionCreateRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[创建群聊会话] 响应失败! reason=" << reason;
                return;
            }

            emit dataCenter->createGroupChatSessionDone();
            LOG() << "[创建群聊会话] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::getMemberList(const QString &loginSessionId, const QString &chatSessionId)
    {
        // 构造请求 body
        Proto::GetChatSessionMemberReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setChatSessionId(chatSessionId);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[获取会话成员列表] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", chatSessionId=" << pbReq.chatSessionId();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/get_chat_session_member", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GetChatSessionMemberRsp>(resp, &ok, &reason);
            if (!ok) {
                LOG() << "[获取会话成员列表] 响应失败 reason=" << reason;
                return;
            }

            dataCenter->resetMemberList(chatSessionId, pbResp->memberInfoList());
            emit dataCenter->getMemberListDone(chatSessionId);
            LOG() << "[获取会话成员列表] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::searchUser(const QString &loginSessionId, const QString &searchKey)
    {
        // 构造请求 body
        Proto::FriendSearchReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setSearchKey(searchKey);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[搜索用户] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << loginSessionId
              << ", searchKey=" << searchKey;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/friend/search_friend", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::FriendSearchRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[搜索用户] 响应失败 reason=" << reason;
                return;
            }

            dataCenter->resetSearchUserResult(pbResp->userInfo());
            emit dataCenter->searchUserDone();
            LOG() << "[搜索用户] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::searchMessage(const QString &loginSessionId, const QString &chatSessionId, const QString &searchKey)
    {
        // 构造请求 body
        Proto::MsgSearchReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setChatSessionId(chatSessionId);
        pbReq.setSearchKey(searchKey);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[按关键词搜索历史消息] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << pbReq.sessionId()
              << ", chatSessionId=" << pbReq.chatSessionId()
              << ", searchKey=" << searchKey;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/message_storage/search_history", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::MsgSearchRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[按关键词搜索历史消息] 响应失败! reason=" << reason;
                return;
            }

            dataCenter->resetSearchMessageResult(pbResp->msgList());
            emit dataCenter->searchMessageDone();
            LOG() << "[按关键词搜索历史消息] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::searchMessageByTime(const QString &loginSessionId, const QString &chatSessionId, const QDateTime &begTime, const QDateTime &endTime)
    {
        // 构造请求 body
        Proto::GetHistoryMsgReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setChatSessionId(chatSessionId);
        pbReq.setStartTime(begTime.toSecsSinceEpoch());
        pbReq.setOverTime(endTime.toSecsSinceEpoch());
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[按时间搜索历史消息] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessionId=" << loginSessionId
              << ", chatSessionId=" << chatSessionId
              << ", begTime=" << begTime
              << ", endTime=" << endTime;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/message_storage/get_history", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GetHistoryMsgRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[按时间搜索历史消息] 响应失败! reason=" << reason;
                return;
            }

            dataCenter->resetSearchMessageResult(pbResp->msgList());
            emit dataCenter->searchMessageDone();
            LOG() << "[按时间搜索历史消息] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::userLogin(const QString &username, const QString &password)
    {
        // 构造请求 body
        Proto::UserLoginReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setNickname(username);
        pbReq.setPassword(password);
        pbReq.setVerifyCodeId("");
        pbReq.setVerifyCode("");
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[用户名登录] 发送请求 requestId=" << pbReq.requestId()
              << ", username=" << pbReq.nickname()
              << ", password=" << pbReq.password();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/username_login", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::UserLoginRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[用户名登录] 处理失败 reason=" << reason;
                emit dataCenter->userLoginDone(false, reason);
                return;
            }

            dataCenter->resetLoginSessionId(pbResp->loginSessionId());
            emit dataCenter->userLoginDone(true, "");
            LOG() << "[用户名登录] 处理响应 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::userRegister(const QString &username, const QString &password)
    {
        // 构造请求 body
        Proto::UserRegisterReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setNickname(username);
        pbReq.setPassword(password);
        pbReq.setVerifyCodeId("");
        pbReq.setVerifyCode("");
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[用户名注册] 发送请求 requestId=" << pbReq.requestId()
              << ", username=" << pbReq.nickname()
              << ", password=" << pbReq.password();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/username_register", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::UserRegisterRsp>(resp, &ok, &reason);
            if (!ok) {
                LOG() << "[用户名注册] 响应失败! reason=" << reason;
                emit dataCenter->userRegisterDone(false, reason);
                return;
            }

            emit dataCenter->userRegisterDone(true, "");
            LOG() << "[用户名注册] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::phoneLogin(const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
    {
        // 构造请求 body
        Proto::PhoneLoginReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setPhoneNumber(phone);
        pbReq.setVerifyCodeId(verifyCodeId);
        pbReq.setVerifyCode(verifyCode);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[手机号登录] 发送请求 requestId=" << pbReq.requestId()
              << ", phone=" << pbReq.phoneNumber()
              << ", verifyCodeId=" << pbReq.verifyCodeId()
              << ", verifyCode=" << pbReq.verifyCode();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/phone_login", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::PhoneLoginRsp>(resp, &ok, &reason);
            if (!ok) {
                LOG() << "[手机号登录] 响应出错! reason=" << reason;
                emit dataCenter->phoneLoginDone(false, reason);
                return;
            }

            dataCenter->resetLoginSessionId(pbResp->loginSessionId());
            emit dataCenter->phoneLoginDone(true, "");
            LOG() << "[手机号登录] 响应完毕 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::phoneRegister(const QString& phone, const QString& verifyCodeId, const QString& verifyCode) {
        // 构造请求 body
        Proto::PhoneRegisterReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setPhoneNumber(phone);
        pbReq.setVerifyCodeId(verifyCodeId);
        pbReq.setVerifyCode(verifyCode);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[手机号注册] 发送请求 requestId=" << pbReq.requestId()
              << ", phone=" << pbReq.phoneNumber()
              << ", verifyCodeId=" << pbReq.verifyCodeId()
              << ", verifyCode=" << pbReq.verifyCode();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/user/phone_register", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::PhoneRegisterRsp>(resp, &ok, &reason);
            if (!ok) {
                LOG() << "[手机号注册] 响应失败! reason=" << reason;
                emit dataCenter->phoneRegisterDone(false, reason);
                return;
            }

            emit dataCenter->phoneRegisterDone(true, "");
            LOG() << "[手机号注册] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::getSingleFile(const QString &loginSessionId, const QString &fileId)
    {
        // 构造请求 body
        Proto::GetSingleFileReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setFileId(fileId);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[获取文件内容] 发送请求 requestId=" << pbReq.requestId()
              << ", fileId=" << fileId;

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/file/get_single_file", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::GetSingleFileRsp>(resp, &ok, &reason);
            if (!ok)
            {
                LOG() << "[获取文件内容] 响应失败 reason=" << reason;
                return;
            }

            emit dataCenter->getSingleFileDone(fileId, pbResp->fileData().fileContent());
            LOG() << "[获取文件内容] 响应完成 requestId=" << pbResp->requestId();
        });
    }

    void NetClient::speechConvertText(const QString &loginSessionId, const QString &fileId, const QByteArray &content)
    {
        // 构造请求 body
        Proto::SpeechRecognitionReq pbReq;
        pbReq.setRequestId(makeRequestId());
        pbReq.setSessionId(loginSessionId);
        pbReq.setSpeechContent(content);
        QByteArray body = pbReq.serialize(&serializer);
        LOG() << "[语音转文字] 发送请求 requestId=" << pbReq.requestId()
              << ", loginSessonId=" << pbReq.sessionId();

        // 发送 HTTP 请求
        QNetworkReply* resp = sendHttpRequest("/service/speech/recognition", body);

        // 处理响应
        connect(resp, &QNetworkReply::finished, this, [=]() {
            bool ok = false;
            QString reason;
            auto pbResp = handleHttpResponse<Proto::SpeechRecognitionRsp>(resp, &ok, &reason);
            if (!ok) {
                LOG() << "[语音转文字] 响应错误! reason=" << reason;
                return;
            }

            emit dataCenter->speechConvertTextDone(fileId, pbResp->recognitionResult());
            LOG() << "[语音转文字] 响应完成 requestId=" << pbResp->requestId();
        });
    }
}
