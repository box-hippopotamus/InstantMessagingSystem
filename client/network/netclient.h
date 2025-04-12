#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QProtobufSerializer>
#include <QNetworkReply>
#include <QUuid>

#include "../model/data.h"

namespace IM::Model
{
class DataCenter;
}

namespace IM::Network
{
    class NetClient : public QObject
    {
        Q_OBJECT
    private:
        const QString HTTP_URL = "http://192.168.229.129:9000";
        const QString WEBSOCKET_URL = "ws://192.168.229.129:9001/ws";

    public:
        NetClient(Model::DataCenter* _dataCenter);

        // 验证网络联通
        void ping();

        // 初始化 websocket
        void initWebsocket();

        // 关闭 wobsocket
        void closeWebsocket();

        // 针对 websocket 的处理
        void handleWsResponse(const Proto::NotifyMessage& notifyMessage);

        void handleWsMessage(const Model::Message& message);

        void handleWsRemoveFriend(const QString& userId);

        void handleWsAddFriendApply(const Model::UserInfo& userInfo);

        void handleWsAddFriendProcess(const Model::UserInfo& userInfo, bool agree);

        void handleWsSessionCreate(const Model::ChatSessionInfo& chatSessionInfo);

        // 发送身份认证请求
        void sendAuth();

        // 生成请求 id
        static QString makeRequestId();

        // 封装发送请求的逻辑
        QNetworkReply* sendHttpRequest(const QString& apiPath, const QByteArray& body);

        // 处理响应
        template <typename T>
        std::shared_ptr<T> handleHttpResponse(QNetworkReply* httpResp, bool* ok, QString* reason)
        {
            if (httpResp->error() != QNetworkReply::NoError)
            {
                *ok = false;
                *reason = httpResp->errorString();
                httpResp->deleteLater();
                return std::shared_ptr<T>();
            }

            // 响应 body
            QByteArray respBody = httpResp->readAll();

            // body 反序列化
            std::shared_ptr<T> respObj = std::make_shared<T>();
            respObj->deserialize(&serializer, respBody);

            // 判定结果是否正确
            if (!respObj->success())
            {
                *ok = false;
                *reason = respObj->errmsg();
                httpResp->deleteLater();
                return std::shared_ptr<T>();
            }

            httpResp->deleteLater();
            *ok = true;
            return respObj;
        }

        void getMyself(const QString& loginSessionId);

        void getFriendList(const QString& loginSessionId);

        void getChatSessionList(const QString& loginSessionId);

        void getApplyList(const QString& loginSessionId);

        void groupAddMember(const QString& loginSessionId,
                            const QString& chatSessionId,
                            const QList<QString>& userIdList);

        void groupModifyName(const QString& loginSessionId,
                             const QString& chatSessionId,
                             const QString& groupName);

        void leaveGroup(const QString& loginSessionId, const QString& chatSessionId);

        void getRecentMessageList(const QString& loginSessionId,
                                  const QString& chatSessionId,
                                  bool updateUI);

        void sendMessage(const QString& loginSessionId,
                         const QString& chatSessionId,
                         Model::MessageType messageType,
                         const QByteArray& content,
                         const QString& extraInfo);

        void receiveMessage(const QString& chatSessionId);

        void changeNickname(const QString& loginSessionId, const QString& nickname);

        void changeDescription(const QString& loginSessionId, const QString& desc);

        void getVerifyCode(const QString& phone);

        void changePhone(const QString& loginSessionId,
                         const QString& phone,
                         const QString& verifyCodeId,
                         const QString& verifyCode);

        void changeAvatar(const QString& loginSessionId, const QByteArray& avatar);

        void deleteFriend(const QString& loginSessionId, const QString& userId);

        void addFriendApply(const QString& loginSessionId, const QString& userId);

        void acceptFriendApply(const QString& loginSessionId, const QString& userId);

        void rejectFriendApply(const QString& loginSessionId, const QString& userId);

        void createGroupChatSession(const QString& loginSessionId, const QList<QString>& userIdList);

        void getMemberList(const QString& loginSessionId, const QString& chatSessionId);

        void searchUser(const QString& loginSessionId, const QString& searchKey);

        void searchMessage(const QString& loginSessionId,
                           const QString& chatSessionId,
                           const QString& searchKey);

        void searchMessageByTime(const QString& loginSessionId,
                                 const QString& chatSessionId,
                                 const QDateTime& begTime,
                                 const QDateTime& endTime);

        void userLogin(const QString& username, const QString& password);

        void userRegister(const QString& username, const QString& password);

        void phoneLogin(const QString& phone,
                        const QString& verifyCodeId,
                        const QString& verifyCode);

        void phoneRegister(const QString& phone,
                           const QString& verifyCodeId,
                           const QString& verifyCode);

        void getSingleFile(const QString& loginSessionId, const QString& fileId);

        void speechConvertText(const QString& loginSessionId,
                               const QString& fileId,
                               const QByteArray& content);

    private:
        Model::DataCenter* dataCenter;
        QNetworkAccessManager httpClient; // http 客户端
        QWebSocket websocketClient; // websocket 客户端
        QProtobufSerializer serializer; // 序列化器
    };
}

#endif // NETCLIENT_H
