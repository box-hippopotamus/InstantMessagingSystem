#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QHttpServer>
#include <QProtobufSerializer>
#include <QWebSocketServer>
#include <QFileInfo>
#include <QFile>
#include <QPixmap>
#include <QIcon>
#include <QDateTime>
#include <QDebug>
#include <QMap>

#include "base.qpb.h"
#include "user.qpb.h"
#include "gateway.qpb.h"
#include "friend.qpb.h"
#include "file.qpb.h"
#include "message_storage.qpb.h"
#include "message_transmit.qpb.h"
#include "speech_recognition.qpb.h"
#include "notify.qpb.h"

static inline QString getFileName(const QString& path)
{
    QFileInfo fileInfo(path);
    return fileInfo.fileName();
}


#define TAG QString("[%1:%2] ").arg(getFileName(__FILE__), QString::number(__LINE__))
#define LOG() qDebug().noquote() << TAG

static inline QString formatTime(int64_t timestamp)
{
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    return dateTime.toString("MM-dd HH:mm:ss");
}

static inline int64_t getTime()
{
    return QDateTime::currentSecsSinceEpoch();
}

// 根据 QByteArray, 转成 QIcon
static inline QIcon makeIcon(const QByteArray& byteArray)
{
    QPixmap pixmap;
    pixmap.loadFromData(byteArray);
    QIcon icon(pixmap);
    return icon;
}

// 从指定文件中, 得到一个 QByteArray
static inline QByteArray loadFileToByteArray(const QString& path)
{
    QFile file(path);
    bool ok = file.open(QFile::ReadOnly);
    if (!ok)
    {
        LOG() << "文件打开失败!";
        return QByteArray();
    }

    QByteArray content = file.readAll();
    file.close();
    return content;
}

// 把 QByteArray 写入文件
static inline void writeByteArrayToFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    bool ok = file.open(QFile::WriteOnly);
    if (!ok)
    {
        LOG() << "文件打开失败!";
        return;
    }

    file.write(content);
    file.flush();
    file.close();
}

// HTTP 服务器
class HttpServer : public QObject
{
    Q_OBJECT

private:
    inline static HttpServer* instance = nullptr;
    HttpServer() = default;

    QHttpServer httpServer;
    QProtobufSerializer serializer;

public:
    static HttpServer* getInstance();

    bool init();

    // 获取个人用户信息
    QHttpServerResponse getUserInfo(const QHttpServerRequest& req);
    // 获取好友列表
    QHttpServerResponse getFriendList(const QHttpServerRequest& req);
    // 获取会话列表
    QHttpServerResponse getChatSessionList(const QHttpServerRequest& req);
    // 获取好友申请列表
    QHttpServerResponse getApplyList(const QHttpServerRequest& req);
    // 获取指定会话的最近消息列表
    QHttpServerResponse getRecent(const QHttpServerRequest& req);
    // 处理发送消息
    QHttpServerResponse newMessage(const QHttpServerRequest& req);
    // 修改用户昵称
    QHttpServerResponse setNickname(const QHttpServerRequest& req);
    // 修改用户签名
    QHttpServerResponse setDesc(const QHttpServerRequest& req);
    // 获取短信验证码
    QHttpServerResponse getPhoneVerifyCode(const QHttpServerRequest& req);
    // 修改手机号
    QHttpServerResponse setPhone(const QHttpServerRequest& req);
    // 修改头像
    QHttpServerResponse setAvatar(const QHttpServerRequest& req);
    // 删除好友
    QHttpServerResponse removeFriend(const QHttpServerRequest& req);
    // 添加好友申请
    QHttpServerResponse addFriendApply(const QHttpServerRequest& req);
    // 添加好友申请的处理
    QHttpServerResponse addFriendProcess(const QHttpServerRequest& req);
    // 创建会话
    QHttpServerResponse createChatSession(const QHttpServerRequest& req);
    // 获取会话成员列表
    QHttpServerResponse getChatSessionMember(const QHttpServerRequest& req);
    // 搜索好友
    QHttpServerResponse searchFriend(const QHttpServerRequest& req);
    // 搜索历史消息
    QHttpServerResponse searchHistory(const QHttpServerRequest& req);
    // 按时间搜索历史消息
    QHttpServerResponse getHistory(const QHttpServerRequest& req);
    // 基于用户名密码登录
    QHttpServerResponse usernameLogin(const QHttpServerRequest& req);
    // 基于用户名密码注册
    QHttpServerResponse usernameRegister(const QHttpServerRequest& req);
    // 手机号登录
    QHttpServerResponse phoneLogin(const QHttpServerRequest& req);
    // 手机号注册
    QHttpServerResponse phoneRegister(const QHttpServerRequest& req);
    // 获取单个文件
    QHttpServerResponse getSingleFile(const QHttpServerRequest& req);
    // 语音转文字
    QHttpServerResponse recognition(const QHttpServerRequest& req);
};


// Websocket
class WebsocketServer : public QObject
{
    Q_OBJECT

private:
    inline static WebsocketServer* instance = nullptr;
    WebsocketServer();

    QWebSocketServer websocketServer;
    QProtobufSerializer serializer;

    int messageIndex = 0;

public:
    static WebsocketServer* getInstance();

    bool init();

signals:
    void sendTextResp();
    void sendImageResp();
    void sendFileResp();
    void sendSpeechResp();
    void sendFriendRemove();
    void sendAddFriendApply();
    void sendAddFriendProcess(bool agree);
    void sendCreateChatSession();
};

#endif // SERVER_H
