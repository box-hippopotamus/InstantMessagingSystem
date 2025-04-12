#include "datacenter.h"

namespace IM::Model
{
    DataCenter::DataCenter()
        : netClient(this)
        , loginSessionId("")
        , currentVerifyCodeId("")
        , currentChatSessionId("")
        , myself(nullptr)
        , applyList(nullptr)
        , friendList(nullptr)
        , chatSessionList(nullptr)
        , searchUserResult(nullptr)
        , searchMessageResult(nullptr)
    {
        recentMessages = new QHash<QString, QList<Message>>();
        memberList = new QHash<QString, QList<UserInfo>>();
        unreadMessageCount = new QHash<QString, int>();

        loadDataFile(); // 加载数据
    }

    DataCenter::~DataCenter()
    {
        delete myself;
        delete friendList;
        delete chatSessionList;
        delete memberList;
        delete applyList;
        delete recentMessages;
        delete unreadMessageCount;
        delete searchUserResult;
        delete searchMessageResult;
    }

    DataCenter *DataCenter::getInstance()
    {
        if (instance == nullptr)
            instance = new DataCenter();

        return instance;
    }

    void DataCenter::initDataFile()
    {
        QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString filePath = basePath + "/ChatClient.json";
        LOG() << "filePath=" << filePath;

        QDir dir;
        if (!dir.exists(basePath))
            dir.mkpath(basePath);

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            LOG() << "打开文件失败!" << file.errorString();
            return;
        }

        // 打开成功, 写入初始内容.
        QString data = "{\n\n}";
        file.write(data.toUtf8());
        file.close();
    }

    void DataCenter::saveDataFile()
    {
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ChatClient.json";

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            LOG() << "文件打开失败! " << file.errorString();
            return;
        }

        QJsonObject jsonObj;
        jsonObj["loginSessionId"] = loginSessionId;

        QJsonObject jsonUnread;
        for (auto it = unreadMessageCount->begin(); it != unreadMessageCount->end(); ++it)
            jsonUnread[it.key()] = it.value();

        jsonObj["unread"] = jsonUnread;

        // 把 json 写入文件
        QJsonDocument jsonDoc(jsonObj);
        QString s = jsonDoc.toJson();
        file.write(s.toUtf8());
        file.close();
    }

    void DataCenter::loadDataFile()
    {
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ChatClient.json";

        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists())
            initDataFile();

        // 读方式打开文件
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            LOG() << "打开文件失败! " << file.errorString();
            return;
        }

        // 读取到文件内容, 解析为 JSON 对象
        QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
        if (jsonDoc.isNull())
        {
            LOG() << "解析 JSON 文件失败! JSON 文件格式有错误!";
            file.close();
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        loginSessionId = jsonObj["loginSessionId"].toString();
        LOG() << "loginSessionId=" << loginSessionId;

        unreadMessageCount->clear();
        QJsonObject jsonUnread = jsonObj["unread"].toObject();
        for (auto it = jsonUnread.begin(); it != jsonUnread.end(); ++it)
            unreadMessageCount->insert(it.key(), it.value().toInt());

        file.close();
    }


    void DataCenter::clearUnread(const QString &chatSessionId)
    {
        (*unreadMessageCount)[chatSessionId] = 0;
        saveDataFile();
    }

    void DataCenter::addUnread(const QString &chatSessionId)
    {
        ++(*unreadMessageCount)[chatSessionId];
        saveDataFile();
    }

    int DataCenter::getUnread(const QString &chatSessionId)
    {
        return (*unreadMessageCount)[chatSessionId];
    }

    const QString &DataCenter::getLoginSessionId() const
    {
        return loginSessionId;
    }

    void DataCenter::ping()
    {
        netClient.ping();
    }

    void DataCenter::initWebsocket()
    {
        netClient.initWebsocket();
    }

    void DataCenter::closeWebsocket()
    {
        netClient.closeWebsocket();
    }

    void DataCenter::getMyselfAsync()
    {
        netClient.getMyself(loginSessionId);
    }

    UserInfo *DataCenter::getMyself()
    {
        return myself;
    }

    void DataCenter::resetMyself(std::shared_ptr<IM::Proto::GetUserInfoRsp> resp)
    {
        if (myself == nullptr)
            myself = new UserInfo();

        const IM::Proto::UserInfo& userInfo = resp->userInfo();
        myself->load(userInfo);
    }

    void DataCenter::getFriendListAsync()
    {
        netClient.getFriendList(loginSessionId);
    }

    QList<UserInfo> *DataCenter::getFriendList()
    {
        return friendList;
    }

    void DataCenter::resetFriendList(std::shared_ptr<IM::Proto::GetFriendListRsp> resp)
    {
        if (friendList == nullptr)
            friendList = new QList<UserInfo>();

        friendList->clear();

        QList<IM::Proto::UserInfo>& friendListPB = resp->friendList();
        for (auto& f : friendListPB)
        {
            UserInfo userInfo;
            userInfo.load(f);
            friendList->push_back(userInfo);
        }
    }

    void DataCenter::getChatSessionListAsync()
    {
        netClient.getChatSessionList(loginSessionId);
    }

    QList<ChatSessionInfo> *DataCenter::getChatSessionList()
    {
        return chatSessionList;
    }

    void DataCenter::leaveGroupAsync(const QString &chatSessionId)
    {
        netClient.leaveGroup(loginSessionId, chatSessionId);
    }

    void DataCenter::deleteChatSession(const QString &chatSessionId)
    {
        for (auto it = chatSessionList->begin(); it!= chatSessionList->end(); ++it)
        {
            if (it->chatSessionId != chatSessionId)
                continue;

            it = chatSessionList->erase(it);
            break;
        }
    }

    void DataCenter::resetChatSessionList(std::shared_ptr<IM::Proto::GetChatSessionListRsp> resp)
    {
        if (chatSessionList == nullptr)
            chatSessionList = new QList<ChatSessionInfo>();

        chatSessionList->clear();

        auto& chatSessionListPB = resp->chatSessionInfoList();
        for (auto& c : chatSessionListPB)
        {
            ChatSessionInfo chatSessionInfo;
            chatSessionInfo.load(c);
            chatSessionList->push_back(chatSessionInfo);
        }
    }

    void DataCenter::getApplyListAsync()
    {
        netClient.getApplyList(loginSessionId);
    }

    QList<UserInfo> *DataCenter::getApplyList()
    {
        return applyList;
    }

    void DataCenter::resetApplyList(std::shared_ptr<IM::Proto::GetPendingFriendEventListRsp> resp)
    {
        if (applyList == nullptr)
            applyList = new QList<UserInfo>();

        applyList->clear();

        auto& eventList = resp->event();
        for (auto& event : eventList)
        {
            UserInfo userInfo;
            userInfo.load(event.sender());
            applyList->push_back(userInfo);
        }
    }

    void DataCenter::getRecentMessageListAsync(const QString &chatSessionId, bool updateUI)
    {
        netClient.getRecentMessageList(loginSessionId, chatSessionId, updateUI);
    }

    QList<Message> *DataCenter::getRecentMessageList(const QString &chatSessionId)
    {
        if (!recentMessages->contains(chatSessionId))
            return nullptr;

        return &(*recentMessages)[chatSessionId];
    }

    void DataCenter::resetRecentMessageList(const QString &chatSessionId, std::shared_ptr<IM::Proto::GetRecentMsgRsp> resp)
    {
        // 并清空 chatSessionId 对应的消息列表,
        QList<Message>& messageList = (*recentMessages)[chatSessionId];
        messageList.clear();

        // 遍历响应结果的列表
        for (auto& m : resp->msgList())
        {
            Message message;
            message.load(m);
            messageList.push_back(message);
        }
    }

    void DataCenter::sendTextMessageAsync(const QString &chatSessionId, const QString &content)
    {
        netClient.sendMessage(loginSessionId, chatSessionId, MessageType::TEXT_TYPE, content.toUtf8(), "");
    }

    void DataCenter::sendImageMessageAsync(const QString &chatSessionId, const QByteArray &content)
    {
        netClient.sendMessage(loginSessionId, chatSessionId, MessageType::IMAGE_TYPE, content, "");
    }

    void DataCenter::sendFileMessageAsync(const QString &chatSessionId, const QString &fileName, const QByteArray &content)
    {
        netClient.sendMessage(loginSessionId, chatSessionId, MessageType::FILE_TYPE, content, fileName);
    }

    void DataCenter::sendSpeechMessageAsync(const QString &chatSessionid, const QByteArray &content)
    {
        netClient.sendMessage(loginSessionId, chatSessionid, MessageType::SPEECH_TYPE, content, "");
    }

    void DataCenter::changeNicknameAsync(const QString &nickname)
    {
        netClient.changeNickname(loginSessionId, nickname);
    }

    void DataCenter::resetNickname(const QString &nickname)
    {
        if (myself == nullptr)
            return;

        myself->nickname = nickname;
    }

    void DataCenter::changeDescriptionAsync(const QString &desc)
    {
        netClient.changeDescription(loginSessionId, desc);
    }

    void DataCenter::resetDescription(const QString &desc)
    {
        if (myself == nullptr)
            return;

        myself->description = desc;
    }

    void DataCenter::getVerifyCodeAsync(const QString &phone)
    {
        netClient.getVerifyCode(phone);
    }

    void DataCenter::resetVerifyCodeId(const QString &verifyCodeId)
    {
        currentVerifyCodeId = verifyCodeId;
    }

    const QString &DataCenter::getVerifyCodeId()
    {
        return currentVerifyCodeId;
    }

    void DataCenter::changePhoneAsync(const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
    {
        netClient.changePhone(loginSessionId, phone, verifyCodeId, verifyCode);
    }

    void DataCenter::resetPhone(const QString &phone)
    {
        if (myself == nullptr)
            return;

        myself->phone = phone;
    }

    void DataCenter::changeAvatarAsync(const QByteArray &imageBytes)
    {
        netClient.changeAvatar(loginSessionId, imageBytes);
    }

    void DataCenter::resetAvatar(const QByteArray &avatar)
    {
        if (myself == nullptr)
            return;

        myself->avatar = makeIcon(avatar);
    }

    void DataCenter::deleteFriendAsync(const QString &userId)
    {
        netClient.deleteFriend(loginSessionId, userId);
    }

    void DataCenter::removeFriend(const QString &userId)
    {
        // 遍历 friendList, 删除其中匹配元素
        if (friendList == nullptr || chatSessionList == nullptr)
            return;

        // QList中，传入lambda，如果该元素的lambda返回true，则删除该元素
        friendList->removeIf([=](const UserInfo& userInfo) {
            return userInfo.userId == userId;
        });

        chatSessionList->removeIf([=](const ChatSessionInfo& chatSessionInfo) {
            if (chatSessionInfo.userId == "" // 群聊，直接跳过
                || chatSessionInfo.userId != userId) // 不是目标会话
                return false;

            if (chatSessionInfo.chatSessionId == currentChatSessionId)
                emit clearCurrentSession(); // 目标会话刚好是当时会话，则清空会话

            return true;
        });
    }

    void DataCenter::addFriendApplyAsync(const QString &userId)
    {
        netClient.addFriendApply(loginSessionId, userId);
    }

    void DataCenter::acceptFriendApplyAsync(const QString &userId)
    {
        netClient.acceptFriendApply(loginSessionId, userId);
    }

    UserInfo DataCenter::removeFromApplyList(const QString &userId)
    {
        if (applyList == nullptr)
            return UserInfo();

        for (auto it = applyList->begin(); it != applyList->end(); ++it)
        {
            if (it->userId != userId)
                continue;

            UserInfo toDelete = *it;
            applyList->erase(it);
            return toDelete;
        }
        return UserInfo();
    }

    void DataCenter::rejectFriendApplyAsync(const QString &userId)
    {
        netClient.rejectFriendApply(loginSessionId, userId);
    }

    void DataCenter::createGroupChatSessionAsync(const QList<QString> &userIdList)
    {
        netClient.createGroupChatSession(loginSessionId, userIdList);
    }

    void DataCenter::groupAddMemberAsync(const QList<QString> &userIdList)
    {
        netClient.groupAddMember(loginSessionId, currentChatSessionId, userIdList);
    }

    void DataCenter::addChatSessionMember(const QString &chatSessionId, const QList<UserInfo> &userList)
    {
        for (const auto& user : userList)
            (*memberList)[chatSessionId].push_back(user);
    }

    void DataCenter::groupModifyNameAsync(const QString &groupName)
    {
        netClient.groupModifyName(loginSessionId, currentChatSessionId, groupName);
    }

    void DataCenter::modifyGroupName(const QString &chatSessionId, const QString &groupName)
    {
        QList<ChatSessionInfo>::iterator it = chatSessionList->begin();
        while (it!= chatSessionList->end())
        {
            if (it->chatSessionId != chatSessionId)
            {
                ++it;
                continue;
            }

            it->chatSessionName = groupName;
            break;
        }
    }

    void DataCenter::getMemberListAsync(const QString &chatSessionId)
    {
        netClient.getMemberList(loginSessionId, chatSessionId);
    }

    QList<UserInfo> *DataCenter::getMemberList(const QString& chatSessionId)
    {
        return memberList->contains(chatSessionId)
                   ? &(*memberList)[chatSessionId]
                   : nullptr;
    }

    void DataCenter::resetMemberList(const QString &chatSessionId, const QList<IM::Proto::UserInfo> &memberList)
    {
        QList<UserInfo>& currentMemberList = (*this->memberList)[chatSessionId];
        currentMemberList.clear();

        for (const auto& m : memberList)
        {
            UserInfo userInfo;
            userInfo.load(m);
            currentMemberList.push_back(userInfo);
        }
    }

    void DataCenter::searchUserAsync(const QString &searchKey)
    {
        netClient.searchUser(loginSessionId, searchKey);
    }

    QList<UserInfo> *DataCenter::getSearchUserResult()
    {
        return searchUserResult;
    }

    void DataCenter::resetSearchUserResult(const QList<IM::Proto::UserInfo> &userList)
    {
        if (searchUserResult == nullptr)
            searchUserResult = new QList<UserInfo>();

        searchUserResult->clear();

        for (const auto& u : userList)
        {
            UserInfo userInfo;
            userInfo.load(u);
            searchUserResult->push_back(userInfo);
        }
    }

    void DataCenter::searchMessageAsync(const QString &searchKey)
    {
        netClient.searchMessage(loginSessionId, this->currentChatSessionId, searchKey);
    }

    void DataCenter::searchMessageByTimeAsync(const QDateTime &begTime, const QDateTime &endTime)
    {
        netClient.searchMessageByTime(loginSessionId, currentChatSessionId, begTime, endTime);
    }

    QList<Message> *DataCenter::getSearchMessageResult()
    {
        return searchMessageResult;
    }

    void DataCenter::resetSearchMessageResult(const QList<Proto::MessageInfo> &msgList)
    {
        if (searchMessageResult == nullptr)
            searchMessageResult = new QList<Message>();

        searchMessageResult->clear();

        for (const auto& m : msgList)
        {
            Message message;
            message.load(m);
            searchMessageResult->push_back(message);
        }
    }

    void DataCenter::userLoginAsync(const QString &username, const QString &password)
    {
        netClient.userLogin(username, password);
    }

    void DataCenter::resetLoginSessionId(const QString &_loginSessionId)
    {
        loginSessionId = _loginSessionId;
        saveDataFile();
    }

    void DataCenter::userRegisterAsync(const QString &username, const QString &password)
    {
        netClient.userRegister(username, password);
    }

    void DataCenter::phoneLoginAsync(const QString &phone, const QString &verifyCode)
    {
        netClient.phoneLogin(phone, currentVerifyCodeId, verifyCode);
    }

    void DataCenter::phoneRegisterAsync(const QString &phone, const QString &verifyCode)
    {
        netClient.phoneRegister(phone, currentVerifyCodeId, verifyCode);
    }

    void DataCenter::getSingleFileAsync(const QString &fileId)
    {
        netClient.getSingleFile(loginSessionId, fileId);
    }

    void DataCenter::speechConvertTextAsync(const QString& fileId, const QByteArray &content)
    {
        netClient.speechConvertText(loginSessionId, fileId, content);
    }

    ChatSessionInfo *DataCenter::findChatSessionById(const QString &chatSessionId)
    {
        if (chatSessionList == nullptr)
            return nullptr;

        for (auto& info : *chatSessionList)
        {
            if (info.chatSessionId == chatSessionId)
                return &info;
        }

        return nullptr;
    }

    ChatSessionInfo *DataCenter::findChatSessionByUserId(const QString &userId)
    {
        if (chatSessionList == nullptr)
            return nullptr;

        for (auto& info : *chatSessionList)
        {
            if (info.userId == userId)
                return &info;
        }

        return nullptr;
    }

    void DataCenter::topChatSessionInfo(const ChatSessionInfo &chatSessionInfo)
    {
        if (chatSessionList == nullptr)
            return;

        // 找到该元素
        auto it = chatSessionList->begin();
        for (; it != chatSessionList->end(); ++it)
        {
            if (it->chatSessionId == chatSessionInfo.chatSessionId)
                break;
        }

        if (it == chatSessionList->end())
            return;

        // 删除元素
        ChatSessionInfo backup = chatSessionInfo;
        chatSessionList->erase(it);

        // 元素插入到头部
        chatSessionList->push_front(backup);
    }

    UserInfo* DataCenter::findFriendById(const QString &userId)
    {
        if (friendList == nullptr)
            return nullptr;

        for (auto& f : *friendList)
        {
            if (f.userId == userId)
                return &f;
        }

        return nullptr;
    }

    void DataCenter::setCurrentChatSessionId(const QString &chatSessionId)
    {
        currentChatSessionId = chatSessionId;
    }

    const QString &DataCenter::getCurrentChatSessionId()
    {
        return currentChatSessionId;
    }

    void DataCenter::addMessage(const Message &message)
    {
        QList<Message>& messageList = (*recentMessages)[message.chatSessionId];
        messageList.push_back(message);
    }
}
