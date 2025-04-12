#include "server.h"

// 辅助函数

// 生成默认的 UserInfo 对象
IM::Proto::UserInfo makeUserInfo(int index, const QByteArray& avatar)
{
    IM::Proto::UserInfo userInfo;
    userInfo.setUserId(QString::number(1000 + index));
    userInfo.setNickname("张三" + QString::number(index));
    userInfo.setDescription("个性签名" + QString::number(index));
    userInfo.setPhone("18612345678");
    userInfo.setAvatar(avatar);
    return userInfo;
}

// 生成默认的 MessageInfo 对象. 文本消息
IM::Proto::MessageInfo makeTextMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar)
{
    IM::Proto::MessageInfo messageInfo;
    messageInfo.setMessageId(QString::number(3000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    IM::Proto::StringMessageInfo stringMessageInfo;
    stringMessageInfo.setContent("这是一条消息内容" + QString::number(index));

    IM::Proto::MessageContent messageContent;
    messageContent.setMessageType(IM::Proto::MessageTypeGadget::MessageType::STRING);
    messageContent.setStringMessage(stringMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

IM::Proto::MessageInfo makeImageMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar)
{
    IM::Proto::MessageInfo messageInfo;
    messageInfo.setMessageId(QString::number(3000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    IM::Proto::ImageMessageInfo imageMessageInfo;
    imageMessageInfo.setFileId("testImage");

    IM::Proto::MessageContent messageContent;
    messageContent.setMessageType(IM::Proto::MessageTypeGadget::MessageType::IMAGE);
    messageContent.setImageMessage(imageMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

IM::Proto::MessageInfo makeFileMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar)
{
    IM::Proto::MessageInfo messageInfo;
    messageInfo.setMessageId(QString::number(3000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    IM::Proto::FileMessageInfo fileMessageInfo;
    fileMessageInfo.setFileId("testFile");
    fileMessageInfo.setFileName("test.txt");
    fileMessageInfo.setFileSize(0);

    IM::Proto::MessageContent messageContent;
    messageContent.setMessageType(IM::Proto::MessageTypeGadget::MessageType::FILE);
    messageContent.setFileMessage(fileMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

IM::Proto::MessageInfo makeSpeechMessageInfo(int index, const QString& chatSessionId, const QByteArray& avatar)
{
    IM::Proto::MessageInfo messageInfo;
    messageInfo.setMessageId(QString::number(3000 + index));
    messageInfo.setChatSessionId(chatSessionId);
    messageInfo.setTimestamp(getTime());
    messageInfo.setSender(makeUserInfo(index, avatar));

    IM::Proto::SpeechMessageInfo speechMessageInfo;
    // 真实服务器推送的消息数据里, 本身也就不带图片的正文. 只是 fileId.
    // 需要通过 fileId, 二次发起请求, 通过 getSingleFile 接口来获取到内容.
    speechMessageInfo.setFileId("testSpeech");

    IM::Proto::MessageContent messageContent;
    messageContent.setMessageType(IM::Proto::MessageTypeGadget::MessageType::SPEECH);
    messageContent.setSpeechMessage(speechMessageInfo);
    messageInfo.setMessage(messageContent);
    return messageInfo;
}

// http
HttpServer *HttpServer::getInstance()
{
    if (instance == nullptr)
        instance = new HttpServer();
    return instance;
}

bool HttpServer::init()
{
    // 返回的值是 int, 表示成功绑定的端口号的数值.
    int ret = httpServer.listen(QHostAddress::Any, 9500);

    // 配置基础路由
    httpServer.route("/ping", [](const QHttpServerRequest& req) -> QHttpServerResponse {
        (void) req;
        qDebug() << "[http] 收到 ping 请求";
        return QHttpServerResponse("pong");
    });

    // 路由表
    const QMap<QString, std::function<QHttpServerResponse(const QHttpServerRequest&)>> routeMap = {
        // 用户相关
        {"/service/user/get_user_info",               [this](const QHttpServerRequest& req) { return getUserInfo(req); }},
        {"/service/user/set_nickname",                [this](const QHttpServerRequest& req) { return setNickname(req); }},
        {"/service/user/set_description",             [this](const QHttpServerRequest& req) { return setDesc(req); }},
        {"/service/user/get_phone_verify_code",       [this](const QHttpServerRequest& req) { return getPhoneVerifyCode(req); }},
        {"/service/user/set_phone",                   [this](const QHttpServerRequest& req) { return setPhone(req); }},
        {"/service/user/set_avatar",                  [this](const QHttpServerRequest& req) { return setAvatar(req); }},
        {"/service/user/username_login",              [this](const QHttpServerRequest& req) { return usernameLogin(req); }},
        {"/service/user/username_register",           [this](const QHttpServerRequest& req) { return usernameRegister(req); }},
        {"/service/user/phone_login",                 [this](const QHttpServerRequest& req) { return phoneLogin(req); }},
        {"/service/user/phone_register",              [this](const QHttpServerRequest& req) { return phoneRegister(req); }},
        // 好友相关
        {"/service/friend/get_friend_list",           [this](const QHttpServerRequest& req) { return getFriendList(req); }},
        {"/service/friend/get_chat_session_list",     [this](const QHttpServerRequest& req) { return getChatSessionList(req); }},
        {"/service/friend/get_pending_friend_events", [this](const QHttpServerRequest& req) { return getApplyList(req); }},
        {"/service/friend/remove_friend",             [this](const QHttpServerRequest& req) { return removeFriend(req); }},
        {"/service/friend/add_friend_apply",          [this](const QHttpServerRequest& req) { return addFriendApply(req); }},
        {"/service/friend/add_friend_process",        [this](const QHttpServerRequest& req) { return addFriendProcess(req); }},
        {"/service/friend/create_chat_session",       [this](const QHttpServerRequest& req) { return createChatSession(req); }},
        {"/service/friend/get_chat_session_member",   [this](const QHttpServerRequest& req) { return getChatSessionMember(req); }},
        {"/service/friend/search_friend",             [this](const QHttpServerRequest& req) { return searchFriend(req); }},
        // 消息相关
        {"/service/message_storage/get_recent",       [this](const QHttpServerRequest& req) { return getRecent(req); }},
        {"/service/message_transmit/new_message",     [this](const QHttpServerRequest& req) { return newMessage(req); }},
        {"/service/message_storage/search_history",   [this](const QHttpServerRequest& req) { return searchHistory(req); }},
        {"/service/message_storage/get_history",      [this](const QHttpServerRequest& req) { return getHistory(req); }},
        // 文件相关
        {"/service/file/get_single_file",             [this](const QHttpServerRequest& req) { return getSingleFile(req); }},
        {"/service/speech/recognition",               [this](const QHttpServerRequest& req) { return recognition(req); }}
    };

    // 注册所有路由
    for (auto it = routeMap.constBegin(); it != routeMap.constEnd(); ++it)
        httpServer.route(it.key(), it.value());

    return ret == 9500;
}

QHttpServerResponse HttpServer::getUserInfo(const QHttpServerRequest &req)
{
    IM::Proto::GetUserInfoReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取用户信息] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 构造数据
    IM::Proto::GetUserInfoRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    IM::Proto::UserInfo userInfo;
    userInfo.setUserId("1029");
    userInfo.setNickname("张三");
    userInfo.setDescription("这是个性签名");
    userInfo.setPhone("18612345678");
    userInfo.setAvatar(loadFileToByteArray(":/resource/image/defaultAvatar.png"));
    pbResp.setUserInfo(userInfo);

    QByteArray body = pbResp.serialize(&serializer);

    // 响应
    QHttpServerResponse httpResp(body, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/x-protobuf");
    return httpResp;
}

QHttpServerResponse HttpServer::getFriendList(const QHttpServerRequest &req)
{
    IM::Proto::GetFriendListReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取好友列表] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 构造响应
    IM::Proto::GetFriendListRsp pbRsp;
    pbRsp.setRequestId(pbReq.requestId());
    pbRsp.setSuccess(true);
    pbRsp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 20; ++i)
    {
        IM::Proto::UserInfo userInfo = makeUserInfo(i, avatar);
        pbRsp.friendList().push_back(userInfo);
    }

    // 序列化
    QByteArray body = pbRsp.serialize(&serializer);

    // 响应
    QHttpServerResponse httpResp(body, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/x-protobuf");
    return httpResp;
}

QHttpServerResponse HttpServer::getChatSessionList(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::GetChatSessionListReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取会话列表] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 构造响应
    IM::Proto::GetChatSessionListRsp pbRsp;
    pbRsp.setRequestId(pbReq.requestId());
    pbRsp.setSuccess(true);
    pbRsp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");

    // 构造单聊会话
    for (int i = 0; i < 30; ++i)
    {
        IM::Proto::ChatSessionInfo chatSessionInfo;
        chatSessionInfo.setChatSessionId(QString::number(2000 + i));
        chatSessionInfo.setChatSessionName("会话" + QString::number(i));
        chatSessionInfo.setSingleChatFriendId(QString::number(1000 + i));
        chatSessionInfo.setAvatar(avatar);
        IM::Proto::MessageInfo messageInfo = makeTextMessageInfo(i, chatSessionInfo.chatSessionId(), avatar);

        chatSessionInfo.setPrevMessage(messageInfo);

        pbRsp.chatSessionInfoList().push_back(chatSessionInfo);
    }

    // 构造一个群聊会话
    QByteArray groupAvatar = loadFileToByteArray(":/resource/image/groupAvatar.png");
    IM::Proto::ChatSessionInfo chatSessionInfo;
    chatSessionInfo.setChatSessionId(QString::number(2100));
    chatSessionInfo.setChatSessionName("会话" + QString::number(2100));
    chatSessionInfo.setSingleChatFriendId("");
    chatSessionInfo.setAvatar(groupAvatar);
    IM::Proto::MessageInfo messageInfo = makeTextMessageInfo(0, chatSessionInfo.chatSessionId(), avatar);
    chatSessionInfo.setPrevMessage(messageInfo);
    pbRsp.chatSessionInfoList().push_back(chatSessionInfo);

    // 序列化响应
    QByteArray body = pbRsp.serialize(&serializer);
    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getApplyList(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::GetPendingFriendEventListReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取好友申请列表] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 构造响应
    IM::Proto::GetPendingFriendEventListRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    // 构造出结果数组
    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 5; ++i)
    {
        IM::Proto::FriendEvent friendEvent;
        friendEvent.setEventId("");
        friendEvent.setSender(makeUserInfo(i, avatar));

        pbResp.event().push_back(friendEvent);
    }

    // 序列化成字节数组
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应对象
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getRecent(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::GetRecentMsgReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取最近消息列表] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId();

    // 构造响应
    IM::Proto::GetRecentMsgRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");

    for (int i = 0; i < 30; ++i)
    {
        IM::Proto::MessageInfo messageInfo = makeTextMessageInfo(i, "2000", avatar);
        pbResp.msgList().push_back(messageInfo);
    }

    IM::Proto::MessageInfo imageMessageInfo = makeImageMessageInfo(30, "2000", avatar);
    pbResp.msgList().push_back(imageMessageInfo);
    IM::Proto::MessageInfo fileMessageInfo = makeFileMessageInfo(31, "2000", avatar);
    pbResp.msgList().push_back(fileMessageInfo);
    IM::Proto::MessageInfo speechMessageInfo = makeSpeechMessageInfo(32, "2000", avatar);
    pbResp.msgList().push_back(speechMessageInfo);

    // 序列化
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应对象
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::newMessage(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::NewMessageReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 发送消息] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId()
          << ", messageType=" << pbReq.message().messageType();

    if (pbReq.message().messageType() == IM::Proto::MessageTypeGadget::MessageType::STRING)
        LOG() << "发送的消息内容=" << pbReq.message().stringMessage().content();

    // 构造响应
    IM::Proto::NewMessageRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setNickname(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::SetUserNicknameReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改用户昵称] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", nickname=" << pbReq.nickname();

    // 构造响应
    IM::Proto::SetUserNicknameRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setDesc(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::SetUserDescriptionReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改用户签名] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", desc=" << pbReq.description();

    // 构造响应
    IM::Proto::SetUserDescriptionRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getPhoneVerifyCode(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::PhoneVerifyCodeReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取短信验证码] requestId=" << pbReq.requestId()
          << ", phone=" << pbReq.phoneNumber();

    // 构造响应 body
    IM::Proto::PhoneVerifyCodeRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setVerifyCodeId("testVerifyCodeId");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setPhone(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::SetUserPhoneNumberReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改手机号] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", phone=" << pbReq.phoneNumber()
          << ", verifyCodeId=" << pbReq.phoneVerifyCodeId()
          << ", verifyCode=" << pbReq.phoneVerifyCode();

    // 构造响应 body
    IM::Proto::SetUserPhoneNumberRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::setAvatar(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::SetUserAvatarReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 修改头像] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId();

    // 构造响应 body
    IM::Proto::SetUserAvatarRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::removeFriend(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::FriendRemoveReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 删除好友] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", peerId=" << pbReq.peerId();

    // 构造响应 body
    IM::Proto::FriendRemoveRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::addFriendApply(const QHttpServerRequest& req) {
    // 解析请求
    IM::Proto::FriendAddReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 添加好友申请] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", userId=" << pbReq.respondentId();

    // 构造响应 body
    IM::Proto::FriendAddRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setNotifyEventId("");

    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::addFriendProcess(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::FriendAddProcessReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 添加好友申请处理] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", applyUserId=" << pbReq.applyUserId()
          << ", agree=" << pbReq.agree();

    // 构造响应 body
    IM::Proto::FriendAddProcessRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setNewSessionId("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::createChatSession(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::ChatSessionCreateReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 创建会话] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", userIdList=" << pbReq.memberIdList();

    // 构造响应 body
    IM::Proto::ChatSessionCreateRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getChatSessionMember(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::GetChatSessionMemberReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取会话成员列表] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId();

    // 构造响应
    IM::Proto::GetChatSessionMemberRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    // 循环的构造多个 userInfo, 添加到 memberInfoList 中
    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 10; ++i)
    {
        IM::Proto::UserInfo userInfo = makeUserInfo(i, avatar);
        pbResp.memberInfoList().push_back(userInfo);
    }

    // 序列化
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::searchFriend(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::FriendSearchReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 搜索好友] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", searchKey=" << pbReq.searchKey();

    // 构造响应 body
    IM::Proto::FriendSearchRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 30; ++i)
    {
        IM::Proto::UserInfo userInfo = makeUserInfo(i, avatar);
        pbResp.userInfo().push_back(userInfo);
    }
    QByteArray body = pbResp.serialize(&serializer);

    // 发送响应给客户端
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::searchHistory(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::MsgSearchReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 搜索历史消息] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId()
          << ", searchKey=" << pbReq.searchKey();

    // 构造响应 body
    IM::Proto::MsgSearchRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 10; ++i)
    {
        IM::Proto::MessageInfo message = makeTextMessageInfo(i, pbReq.chatSessionId(), avatar);
        pbResp.msgList().push_back(message);
    }

    // 构造图片消息
    IM::Proto::MessageInfo message = makeImageMessageInfo(10, pbReq.chatSessionId(), avatar);
    pbResp.msgList().push_back(message);
    // 构造文件消息
    message = makeFileMessageInfo(11, pbReq.chatSessionId(), avatar);
    pbResp.msgList().push_back(message);
    // 构造语音消息
    message = makeSpeechMessageInfo(12, pbReq.chatSessionId(), avatar);
    pbResp.msgList().push_back(message);

    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::getHistory(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::GetHistoryMsgReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 按时间搜索历史消息] requestId=" << pbReq.requestId()
          << ", loginSessionId=" << pbReq.sessionId()
          << ", chatSessionId=" << pbReq.chatSessionId()
          << ", begTime=" << pbReq.startTime()
          << ", endTime=" << pbReq.overTime();

    // 构造响应
    IM::Proto::GetHistoryMsgRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 10; ++i)
    {
        IM::Proto::MessageInfo message = makeTextMessageInfo(i, pbReq.chatSessionId(), avatar);
        pbResp.msgList().push_back(message);
    }
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::usernameLogin(const QHttpServerRequest& req)
{
    // 解析请求
    IM::Proto::UserLoginReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 用户名密码登录] requestId=" << pbReq.requestId()
          << ", username=" << pbReq.nickname()
          << ", password=" << pbReq.password();

    // 构造响应 body
    IM::Proto::UserLoginRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setLoginSessionId("testLoginSessionId");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::usernameRegister(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::UserRegisterReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 用户名密码注册] requestId=" << pbReq.requestId()
          << ", username=" << pbReq.nickname()
          << ", password=" << pbReq.password();

    // 构造响应 body
    IM::Proto::UserRegisterRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QString body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::phoneLogin(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::PhoneLoginReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 手机号登录] requestId=" << pbReq.requestId()
          << ", phone=" << pbReq.phoneNumber()
          << ", verifyCodeId=" << pbReq.verifyCodeId()
          << ", verifyCode=" << pbReq.verifyCode();

    // 构造响应
    IM::Proto::PhoneLoginRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setLoginSessionId("testLoginSessionId");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::phoneRegister(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::PhoneRegisterReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 手机号注册] requestId=" << pbReq.requestId()
          << ", phone=" << pbReq.phoneNumber()
          << ", verifyCodeId=" << pbReq.verifyCodeId()
          << ", verifyCode=" << pbReq.verifyCode();

    // 构造响应 body
    IM::Proto::PhoneRegisterRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    return resp;
}

QHttpServerResponse HttpServer::getSingleFile(const QHttpServerRequest &req)
{
    // 解析请求
    IM::Proto::GetSingleFileReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 获取单个文件] requestId=" << pbReq.requestId()
          << ", fileId=" << pbReq.fileId();

    // 构造响应 body
    IM::Proto::GetSingleFileRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");

    IM::Proto::FileDownloadData fileDownloadData;
    fileDownloadData.setFileId(pbReq.fileId());

    // 直接使用 fileId 做区分
    if (pbReq.fileId() == "testImage")
    {
        fileDownloadData.setFileContent(loadFileToByteArray(":/resource/image/defaultAvatar.png"));
    }
    else if (pbReq.fileId() == "testFile")
    {
        fileDownloadData.setFileContent(loadFileToByteArray(":/resource/file/test.txt"));
    }
    else if (pbReq.fileId() == "testSpeech")
    {
        fileDownloadData.setFileContent(loadFileToByteArray(":/resource/file/speech.pcm"));
    } else
    {
        pbResp.setSuccess(false);
        pbResp.setErrmsg("fileId 不是预期的测试 fileId");
    }
    pbResp.setFileData(fileDownloadData);

    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-Type", "application/x-protobuf");
    return resp;
}

QHttpServerResponse HttpServer::recognition(const QHttpServerRequest &req)
{
    // 解析请求 body
    IM::Proto::SpeechRecognitionReq pbReq;
    pbReq.deserialize(&serializer, req.body());
    LOG() << "[REQ 语音转文字] requestId=" << pbReq.requestId() << ", loginSessionId=" << pbReq.sessionId();

    // 构造响应 body
    IM::Proto::SpeechRecognitionRsp pbResp;
    pbResp.setRequestId(pbReq.requestId());
    pbResp.setSuccess(true);
    pbResp.setErrmsg("");
    pbResp.setRecognitionResult("你好你好, 这是一段语音消息, 你好你好, 这是一段语音消息");
    QByteArray body = pbResp.serialize(&serializer);

    // 构造 HTTP 响应
    QHttpServerResponse resp(body, QHttpServerResponse::StatusCode::Ok);
    resp.setHeader("Content-type", "application/x-protobuf");
    return resp;
}


// websocket
WebsocketServer::WebsocketServer()
    : websocketServer("websocket server", QWebSocketServer::NonSecureMode)
{}

WebsocketServer *WebsocketServer::getInstance()
{
    if (instance == nullptr)
        instance = new WebsocketServer();

    return instance;
}

bool WebsocketServer:: init()
{
    // 连接信号槽
    connect(&websocketServer, &QWebSocketServer::newConnection, this, [this]() {
        qDebug() << "[websocket] 连接建立成功!";

        // 获取到用来通信的 socket 对象. nextPendingConnection 类似于 原生 socket 中的 accept
        QWebSocket* socket = websocketServer.nextPendingConnection();

        // 针对这个 socket 对象, 进行剩余信号的处理
        connect(socket, &QWebSocket::disconnected, this, [=]() {
            qDebug() << "[websocket] 连接断开!";
            disconnect(this, &WebsocketServer::sendTextResp, this, nullptr);
            disconnect(this, &WebsocketServer::sendFriendRemove, this, nullptr);
            disconnect(this, &WebsocketServer::sendAddFriendApply, this, nullptr);
            disconnect(this, &WebsocketServer::sendAddFriendProcess, this, nullptr);
            disconnect(this, &WebsocketServer::sendCreateChatSession, this, nullptr);
            disconnect(this, &WebsocketServer::sendImageResp, this, nullptr);
            disconnect(this, &WebsocketServer::sendFileResp, this, nullptr);
            disconnect(this, &WebsocketServer::sendSpeechResp, this, nullptr);
        });

        connect(socket, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error) {
            qDebug() << "[websocket] 连接出错! " << error;
        });

        connect(socket, &QWebSocket::textMessageReceived, this, [=](const QString& message) {
            qDebug() << "[websocket] 收到文本数据! message=" << message;
        });

        connect(socket, &QWebSocket::binaryMessageReceived, this, [=](const QByteArray& byteArray) {
            qDebug() << "[websocket] 收到二进制数据! " << byteArray.length();
        });

        connect(this, &WebsocketServer::sendTextResp, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效!";
                return;
            }

            QByteArray avatar = loadFileToByteArray(":/resouce/image/defaultAvatar.png");
            IM::Proto::MessageInfo messageInfo = makeTextMessageInfo(messageIndex++, "2000", avatar);

            IM::Proto::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);

            QByteArray body = notifyMessage.serialize(&this->serializer); // 序列化
            socket->sendBinaryMessage(body); // 发送消息给客户端
            LOG() << "发送文本消息响应: " << messageInfo.message().stringMessage().content();
        });

        connect(this, &WebsocketServer::sendImageResp, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效!";
                return;
            }

            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/resouce/image/defaultAvatar.png");
            IM::Proto::MessageInfo messageInfo = makeImageMessageInfo(messageIndex++, "2000", avatar);

            IM::Proto::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);


            QByteArray body = notifyMessage.serialize(&this->serializer); // 序列化
            socket->sendBinaryMessage(body); // 发送消息给客户端
            LOG() << "发送图片消息响应";
        });

        connect(this, &WebsocketServer::sendFileResp, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效!";
                return;
            }

            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
            IM::Proto::MessageInfo messageInfo = makeFileMessageInfo(messageIndex++, "2000", avatar);

            IM::Proto::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);


            QByteArray body = notifyMessage.serialize(&this->serializer); // 序列化
            socket->sendBinaryMessage(body); // 发送消息给客户端
            LOG() << "发送文件消息响应";
        });

        connect(this, &WebsocketServer::sendSpeechResp, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效!";
                return;
            }

            // 构造响应数据
            QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
            IM::Proto::MessageInfo messageInfo = makeSpeechMessageInfo(messageIndex++, "2000", avatar);

            IM::Proto::NotifyNewMessage notifyNewMessage;
            notifyNewMessage.setMessageInfo(messageInfo);

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::CHAT_MESSAGE_NOTIFY);
            notifyMessage.setNewMessageInfo(notifyNewMessage);


            QByteArray body = notifyMessage.serialize(&serializer); // 序列化
            socket->sendBinaryMessage(body); // 发送消息给客户端
            LOG() << "发送语音消息响应";
        });

        connect(this, &WebsocketServer::sendFriendRemove, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效";
                return;
            }

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::FRIEND_REMOVE_NOTIFY);

            IM::Proto::NotifyFriendRemove notifyFriendRemove;
            notifyFriendRemove.setUserId("1000");
            notifyMessage.setFriendRemove(notifyFriendRemove);

            QByteArray body = notifyMessage.serialize(&serializer);
            socket->sendBinaryMessage(body);
            LOG() << "通知对方好友被删除 userId=1000";
        });

        connect(this, &WebsocketServer::sendAddFriendApply, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效";
                return;
            }

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::FRIEND_ADD_APPLY_NOTIFY);

            QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
            IM::Proto::UserInfo userInfo = makeUserInfo(100, avatar);

            IM::Proto::NotifyFriendAddApply friendAddApply;
            friendAddApply.setUserInfo(userInfo);

            notifyMessage.setFriendAddApply(friendAddApply);

            QByteArray body = notifyMessage.serialize(&serializer);
            socket->sendBinaryMessage(body);
            LOG() << "通知对方好友申请数据";
        });

        connect(this, &WebsocketServer::sendAddFriendProcess, this, [=](bool agree) {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效!";
                return;
            }

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::FRIEND_ADD_PROCESS_NOTIFY);

            QByteArray avatar = loadFileToByteArray(":/resource/image/defaultAvatar.png");
            IM::Proto::UserInfo userInfo = makeUserInfo(100, avatar);

            IM::Proto::NotifyFriendAddProcess friendAddProcess;
            friendAddProcess.setUserInfo(userInfo);
            friendAddProcess.setAgree(agree);

            notifyMessage.setFriendProcessResult(friendAddProcess);

            QByteArray body = notifyMessage.serialize(&serializer);
            socket->sendBinaryMessage(body);
            LOG() << "通知好友申请的处理结果 userId=" << userInfo.userId() << ", agree=" << agree;
        });

        connect(this, &WebsocketServer::sendCreateChatSession, this, [=]() {
            if (socket == nullptr || !socket->isValid())
            {
                LOG() << "socket 对象无效!";
                return;
            }

            QByteArray avatar = loadFileToByteArray(":/resource/image/groupAvatar.png");

            IM::Proto::NotifyMessage notifyMessage;
            notifyMessage.setNotifyEventId("");
            notifyMessage.setNotifyType(IM::Proto::NotifyTypeGadget::NotifyType::CHAT_SESSION_CREATE_NOTIFY);

            IM::Proto::MessageInfo messageInfo = makeTextMessageInfo(0, "2100", avatar);

            IM::Proto::ChatSessionInfo chatSessionInfo;
            chatSessionInfo.setChatSessionId("2100");
            chatSessionInfo.setSingleChatFriendId("");
            chatSessionInfo.setChatSessionName("新的群聊");
            chatSessionInfo.setPrevMessage(messageInfo);
            chatSessionInfo.setAvatar(avatar);

            IM::Proto::NotifyNewChatSession newChatSession;
            newChatSession.setChatSessionInfo(chatSessionInfo);
            notifyMessage.setNewChatSessionInfo(newChatSession);


            QByteArray body = notifyMessage.serialize(&serializer); // 序列化操作
            socket->sendBinaryMessage(body); // 通过 websocket 推送数据
            LOG() << "通知创建会话!";
        });
    });

    // 绑定端口, 启动服务
    bool ok = websocketServer.listen(QHostAddress::Any, 9501);
    return ok;
}

