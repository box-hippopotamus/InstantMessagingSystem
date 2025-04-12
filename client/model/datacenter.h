#ifndef DATACENTER_H
#define DATACENTER_H

#include <QWidget>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>

#include "data.h"
#include "../network/netclient.h"

namespace IM::Model
{
    class DataCenter : public QObject
    {
        Q_OBJECT

    public:
        static DataCenter* getInstance();
        ~DataCenter();

    private:
        DataCenter();

        inline static DataCenter* instance = nullptr;

        QString loginSessionId; // 登录会话 id
        UserInfo* myself; // 当前的用户信息
        QList<UserInfo>* friendList; // 好友列表
        QList<ChatSessionInfo>* chatSessionList; // 会话列表
        QString currentChatSessionId; // 记录当前选中的会话
        QList<UserInfo>* applyList; // 待处理的好友申请列表
        QList<UserInfo>* searchUserResult; // 好友搜索结果
        QList<Message>* searchMessageResult; // 历史消息搜索结果
        QString currentVerifyCodeId; // 短信验证码的验证 id
        Network::NetClient netClient; // NetClient 实例

        //  chatSessionId   成员列表
        QHash<QString, QList<UserInfo>>* memberList; // 每个会话的成员

        //  chatSessionId   消息列表
        QHash<QString, QList<Message>>* recentMessages; // 每个会话的最近消息列表

        //  chatSessionId, 未读消息的个数
        QHash<QString, int>* unreadMessageCount; // 每个会话,未读消息个数

    public:
        // 初始化数据文件
        void initDataFile();
        // 存储数据到文件中
        void saveDataFile();
        // 从数据文件中加载数据到内存
        void loadDataFile();
        // 清空未读消息数目
        void clearUnread(const QString& chatSessionId);
        // 增加未读消息数目
        void addUnread(const QString& chatSessionId);
        // 获取未读消息数目
        int getUnread(const QString& chatSessionId);

        // 获取到当前的登录会话id
        const QString& getLoginSessionId() const;

        // 验证网络连通性
        void ping();

        // 针对 netclient 中的 websocket 进行初始化.
        void initWebsocket();
        void closeWebsocket();

        // 通过网络获取到用户的个人信息
        void getMyselfAsync();
        UserInfo* getMyself();
        void resetMyself(std::shared_ptr<Proto::GetUserInfoRsp> resp);

        // 获取好友列表
        void getFriendListAsync();
        QList<UserInfo>* getFriendList();
        void resetFriendList(std::shared_ptr<Proto::GetFriendListRsp> resp);

        // 获取会话列表
        void getChatSessionListAsync();
        QList<ChatSessionInfo>* getChatSessionList();
        void resetChatSessionList(std::shared_ptr<Proto::GetChatSessionListRsp> resp);

        // 删除会话列表的一个会话
        void leaveGroupAsync(const QString& chatSessionId);
        void deleteChatSession(const QString& chatSessionId);

        // 获取好友申请列表
        void getApplyListAsync();
        QList<UserInfo>* getApplyList();
        void resetApplyList(std::shared_ptr<Proto::GetPendingFriendEventListRsp> resp);

        // 获取最近消息列表
        void getRecentMessageListAsync(const QString& chatSessionId, bool updateUI);
        QList<Message>* getRecentMessageList(const QString& chatSessionId);
        void resetRecentMessageList(const QString& chatSessionId, std::shared_ptr<IM::Proto::GetRecentMsgRsp> resp);

        // 发送消息给服务器
        void sendTextMessageAsync(const QString& chatSessionId, const QString& content);
        void sendImageMessageAsync(const QString& chatSessionId, const QByteArray& content);
        void sendFileMessageAsync(const QString& chatSessionId, const QString& fileName, const QByteArray& content);
        void sendSpeechMessageAsync(const QString& chatSessionid, const QByteArray& content);

        // 修改用户昵称
        void changeNicknameAsync(const QString& nickname);
        void resetNickname(const QString& nickname);

        // 修改用户签名
        void changeDescriptionAsync(const QString& desc);
        void resetDescription(const QString& desc);

        // 获取短信验证码
        void getVerifyCodeAsync(const QString& phone);
        void resetVerifyCodeId(const QString& verifyCodeId);
        // 获取 verifyCodeId
        const QString& getVerifyCodeId();

        // 修改手机号码
        void changePhoneAsync(const QString& phone, const QString& verifyCodeId, const QString& verifyCode);
        void resetPhone(const QString& phone);

        // 修改头像
        void changeAvatarAsync(const QByteArray& imageBytes);
        void resetAvatar(const QByteArray& avatar);

        // 删除好友
        void deleteFriendAsync(const QString& userId);
        void removeFriend(const QString& userId);

        // 发送好友申请
        void addFriendApplyAsync(const QString& userId);

        // 发送同意好友申请操作
        void acceptFriendApplyAsync(const QString& userId);
        UserInfo removeFromApplyList(const QString& userId);

        // 拒绝好友申请操作
        void rejectFriendApplyAsync(const QString& userId);

        // 创建群聊
        void createGroupChatSessionAsync(const QList<QString>& userIdList);

        // 添加群聊成员
        void groupAddMemberAsync(const QList<QString>& userIdList);
        void addChatSessionMember(const QString &chatSessionId, const QList<UserInfo> &userList);

        // 修改群聊名称
        void groupModifyNameAsync(const QString& groupName);
        void modifyGroupName(const QString& chatSessionId, const QString& groupName);

        // 获取会话的成员列表
        void getMemberListAsync(const QString& chatSessionId);
        QList<UserInfo>* getMemberList(const QString& chatSessionId);
        void resetMemberList(const QString& chatSessionId, const QList<Proto::UserInfo>& memberList);

        // 搜索用户
        void searchUserAsync(const QString& searchKey);
        QList<UserInfo>* getSearchUserResult();
        void resetSearchUserResult(const QList<Proto::UserInfo>& userList);

        // 搜索历史消息
        void searchMessageAsync(const QString& searchKey);
        void searchMessageByTimeAsync(const QDateTime& begTime, const QDateTime& endTime);
        QList<Message>* getSearchMessageResult();
        void resetSearchMessageResult(const QList<Proto::MessageInfo>& msgList);

        // 登录注册
        void userLoginAsync(const QString& username, const QString& password);
        void resetLoginSessionId(const QString& loginSessionId);
        void userRegisterAsync(const QString& username, const QString& password);
        void phoneLoginAsync(const QString& phone, const QString& verifyCode);
        void phoneRegisterAsync(const QString& phone, const QString& verifyCode);

        // 获取单个文件
        void getSingleFileAsync(const QString& fileId);

        // 语音转文字
        void speechConvertTextAsync(const QString& fileId, const QByteArray& content);

    public:
        // 根据会话 id 查询会话信息
        ChatSessionInfo* findChatSessionById(const QString& chatSessionId);
        // 根据用户 id 查询会话信息
        ChatSessionInfo* findChatSessionByUserId(const QString& userId);
        // 把指定的会话信息, 放到列表头部.
        void topChatSessionInfo(const ChatSessionInfo& chatSessionInfo);
        // 根据用户 id 查询好友信息
        UserInfo* findFriendById(const QString& userId);

        // 设置/获取当前选中的会话
        void setCurrentChatSessionId(const QString& chatSessionId);
        const QString& getCurrentChatSessionId();

        // 添加消息到 DataCenter 中
        void addMessage(const Message& message);

    signals:
        void getMyselfDone();
        void getFriendListDone();
        void getChatSessionListDone();
        void getApplyListDone();
        void groupAddMemberDone();
        void groupModifyNameDone(const QString &chatSessionId, const QString& groupName);
        void leaveGroupDone(const QString &chatSessionId);
        void getRecentMessageListDone(const QString& chatSessionId);
        void getRecentMessageListDoneNoUI(const QString& chatSessionId);
        void sendMessageDone(MessageType messageType, const QByteArray& content, const QString& extraInfo);
        void updateLastMessage(const QString& chatSessionId);
        void receiveMessageDone(const Message& lastMessage);
        void changeNicknameDone();
        void changeDescriptionDone();
        void getVerifyCodeDone();
        void changePhoneDone();
        void changeAvatarDone();
        void deleteFriendDone();
        void clearCurrentSession();
        void addFriendApplyDone();
        void receiveFriendApplyDone();
        void acceptFriendApplyDone();
        void rejectFriendApplyDone();
        void receiveFriendProcessDone(const QString& nickname, bool agree);
        void createGroupChatSessionDone();
        void receiveSessionCreateDone();
        void getMemberListDone(const QString& chatSessionId);
        void searchUserDone();
        void searchMessageDone();
        void userLoginDone(bool ok, const QString& reason);
        void userRegisterDone(bool ok, const QString& reason);
        void phoneLoginDone(bool ok, const QString& reason);
        void phoneRegisterDone(bool ok, const QString& reason);
        void getSingleFileDone(const QString& fileId, const QByteArray& fileContent);
        void speechConvertTextDone(const QString& fileId, const QString& text);
    };
}

#endif // DATACENTER_H
