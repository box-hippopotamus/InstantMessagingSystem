#include "userinfowidget.h"
#include "mainwidget.h"

using namespace IM::Model;

UserInfoWidget::UserInfoWidget(const UserInfo& userInfo, QWidget* parent)
    : QDialog(parent)
    , userInfo(userInfo)
{
    // 设置基本属性
    setFixedSize(350, 200);
    setWindowTitle("用户详情");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setAttribute(Qt::WA_DeleteOnClose);
    move(QCursor::pos());

    // 设置窗口背景色
    setStyleSheet("QDialog { background-color: rgb(245, 245, 245); }");

    // 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setVerticalSpacing(10);
    layout->setHorizontalSpacing(20);
    layout->setContentsMargins(30, 20, 30, 20);
    layout->setAlignment(Qt::AlignTop);
    setLayout(layout);

    // 头像样式
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(80, 80);
    avatarBtn->setIconSize(QSize(80, 80));
    avatarBtn->setIcon(userInfo.avatar);
    avatarBtn->setStyleSheet(
        "QPushButton {"
        " border: 2px solid rgb(208, 221, 247);"
        " border-radius: 5px;"
        " background-color: white;"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(240, 243, 250);"
        "}");

    // 标签样式
    static const QString labelStyle = "QLabel {"
                                      " font-size: 14px;"
                                      " font-weight: 600;"
                                      " color: rgb(80, 80, 80);"
                                      " padding-left: 10px;"
                                      "}";

    // 内容样式
    static const QString contentStyle = "QLabel {"
                                        " font-size: 14px;"
                                        " color: rgb(120, 120, 120);"
                                        "}";

    // 按钮基础样式
    static const QString btnStyle = "QPushButton {"
                                    " border: 1px solid rgb(208, 221, 247);"
                                    " border-radius: 4px;"
                                    " padding: 5px;"
                                    " background-color: rgb(240, 243, 250);"
                                    " color: rgb(100, 130, 180);"
                                    "}"
                                    "QPushButton:pressed {"
                                    " background-color: rgb(208, 221, 247);"
                                    "}";

    // 删除按钮样式
    static const QString deleteBtnStyle = "QPushButton {"
                                          " border: 1px solid rgb(240, 150, 150);"
                                          " border-radius: 4px;"
                                          " background-color: rgb(250, 230, 230);"
                                          " color: rgb(180, 80, 80);"
                                          "}"
                                          "QPushButton:pressed {"
                                          " background-color: rgb(240, 150, 150);"
                                          "}";

    static const int width = 60;
    static const int height = 30;

    // 用户ID
    idTag = new QLabel("ID");
    idTag->setStyleSheet(labelStyle);
    idTag->setFixedSize(width, height);

    idLabel = new QLabel(userInfo.userId);
    idLabel->setStyleSheet(contentStyle);
    idLabel->setFixedHeight(height);

    // 用户昵称
    nameTag = new QLabel("昵称");
    nameTag->setStyleSheet(labelStyle);
    nameTag->setFixedSize(width, height);

    nameLabel = new QLabel(userInfo.nickname);
    nameLabel->setStyleSheet(contentStyle);
    nameLabel->setFixedHeight(height);

    // 电话
    phoneTag = new QLabel("电话");
    phoneTag->setStyleSheet(labelStyle);
    phoneTag->setFixedSize(width, height);

    phoneLabel = new QLabel(userInfo.phone);
    phoneLabel->setStyleSheet(contentStyle);
    phoneLabel->setFixedHeight(height);

    // 功能按钮
    applyBtn = new QPushButton("申请好友");
    applyBtn->setFixedSize(90, 30);
    applyBtn->setStyleSheet(btnStyle);

    sendMessageBtn = new QPushButton("发送消息");
    sendMessageBtn->setFixedSize(90, 30);
    sendMessageBtn->setStyleSheet(btnStyle);

    deleteFriendBtn = new QPushButton("删除好友");
    deleteFriendBtn->setFixedSize(90, 30);
    deleteFriendBtn->setStyleSheet(deleteBtnStyle);

    // 添加到布局管理器
    layout->addWidget(avatarBtn, 0, 0, 3, 1, Qt::AlignCenter);

    layout->addWidget(idTag, 0, 1);
    layout->addWidget(idLabel, 0, 2, 1, 2);

    layout->addWidget(nameTag, 1, 1);
    layout->addWidget(nameLabel, 1, 2, 1, 2);

    layout->addWidget(phoneTag, 2, 1);
    layout->addWidget(phoneLabel, 2, 2, 1, 2);

    // 按钮行使用单独的布局保持间距一致
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(sendMessageBtn);
    btnLayout->addWidget(deleteFriendBtn);
    btnLayout->addStretch();

    layout->addLayout(btnLayout, 3, 0, 1, 4);

    // 按钮禁用关系
    DataCenter* dataCenter = DataCenter::getInstance();
    auto* myFriend = dataCenter->findFriendById(userInfo.userId);
    if (myFriend == nullptr)
    {
        // sendMessageBtn->setEnabled(false);
        // deleteFriendBtn->setEnabled(false);
        sendMessageBtn->hide();
        deleteFriendBtn->hide();
    }
    else
    {
        // applyBtn->setEnabled(false);
        applyBtn->hide();
    }

    // 初始化信号槽
    initSignalSlot();
}

void UserInfoWidget::initSignalSlot()
{
    connect(sendMessageBtn, &QPushButton::clicked, this, [=]() {
        MainWidget* mainWidget = MainWidget::getInstance();
        mainWidget->switchSession(userInfo.userId);
        close();
    });

    connect(deleteFriendBtn, &QPushButton::clicked, this, &UserInfoWidget::clickDeleteFriendBtn);
    connect(applyBtn, &QPushButton::clicked, this, &UserInfoWidget::clickApplyBtn);
}

void UserInfoWidget::clickDeleteFriendBtn()
{
    // 用户再次确认是否删除
    auto result = QMessageBox::warning(this, "确认删除", "确认删除当前好友?", QMessageBox::Ok | QMessageBox::Cancel);
    if (result != QMessageBox::Ok)
    {
        LOG() << "删除好友取消";
        return;
    }

    // 发送请求
    DataCenter* dataCenter = DataCenter::getInstance();
    dataCenter->deleteFriendAsync(userInfo.userId);
    // 关闭窗口
    close();
}

void UserInfoWidget::clickApplyBtn()
{
    // 发送好友申请
    DataCenter* dataCenter = DataCenter::getInstance();
    dataCenter->addFriendApplyAsync(userInfo.userId);

    // 关闭窗口
    close();
}
