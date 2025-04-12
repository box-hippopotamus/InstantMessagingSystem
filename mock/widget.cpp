#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Instant Message System Mock Server");
    resize(400, 400);

    // 水平布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(10);

    // 发送消息
    QPushButton* sendMessage = new QPushButton(this);
    sendMessage->setText("发送消息");

    connect(sendMessage, &QPushButton::clicked, this, [](){
        WebsocketServer* ws = WebsocketServer::getInstance();
        emit ws->sendTextResp();
    });

    // 删除好友
    QPushButton* sendFriendRemove = new QPushButton(this);
    sendFriendRemove->setText("删除好友");

    connect(sendFriendRemove, &QPushButton::clicked, this, [](){
        WebsocketServer* ws = WebsocketServer::getInstance();
        emit ws->sendFriendRemove();
    });

    // 添加好友申请
    QPushButton* sendAddFriendApply = new QPushButton(this);
    sendAddFriendApply->setText("添加好友申请");

    connect(sendAddFriendApply, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendAddFriendApply();
    });

    // 同意好友申请
    QPushButton* sendAcceptFriendApply = new QPushButton(this);
    sendAcceptFriendApply->setText("同意好友申请");

    connect(sendAcceptFriendApply, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendAddFriendProcess(true);
    });

    // 拒绝好友申请
    QPushButton* sendRejectFriendApply = new QPushButton(this);
    sendRejectFriendApply->setText("拒绝好友申请");

    connect(sendRejectFriendApply, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendAddFriendProcess(false);
    });

    // 创建会话
    QPushButton* sendCreateChatSession = new QPushButton(this);
    sendCreateChatSession->setText("创建会话");

    connect(sendCreateChatSession, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendCreateChatSession();
    });

    // 发送图片消息
    QPushButton* sendImageMessage = new QPushButton(this);
    sendImageMessage->setText("发送图片消息");

    connect(sendImageMessage, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendImageResp();
    });

    // 发送文件消息
    QPushButton* sendFileMessage = new QPushButton(this);
    sendFileMessage->setText("发送文件消息");

    connect(sendFileMessage, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendFileResp();
    });

    // 发送语音消息
    QPushButton* sendSpeechMessage = new QPushButton(this);
    sendSpeechMessage->setText("发送语音消息");

    connect(sendSpeechMessage, &QPushButton::clicked, this, [](){
        WebsocketServer* websocketServer = WebsocketServer::getInstance();
        emit websocketServer->sendSpeechResp();
    });

    // 将按钮添加到水平布局中
    buttonLayout->addWidget(sendMessage);
    buttonLayout->addWidget(sendFriendRemove);
    buttonLayout->addWidget(sendAddFriendApply);
    buttonLayout->addWidget(sendAcceptFriendApply);
    buttonLayout->addWidget(sendRejectFriendApply);
    buttonLayout->addWidget(sendCreateChatSession);
    buttonLayout->addWidget(sendImageMessage);
    buttonLayout->addWidget(sendFileMessage);
    buttonLayout->addWidget(sendSpeechMessage);

    setLayout(buttonLayout);
}

Widget::~Widget()
{
    delete ui;
}
