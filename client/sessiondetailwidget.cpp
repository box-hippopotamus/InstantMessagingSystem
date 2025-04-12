#include "sessiondetailwidget.h"

AvatarItem::AvatarItem(const QIcon &avatar, const QString &name)
{
    // 基本属性
    setFixedSize(70, 80);

    // 布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignHCenter);
    setLayout(layout);

    // 头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(45, 45);
    avatarBtn->setIconSize(QSize(45, 45));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet(
        "QPushButton {"
        " border: none;"
        " border-radius: 5px;"
        " background-color: transparent;"
        "}"
        "QPushButton:hover {"
        " background-color: rgba(208, 221, 247, 0.3);"
        "}"
        "QPushButton:pressed {"
        " background-color: rgba(208, 221, 247, 0.5);"
        "}");

    // 名字
    nameLabel = new QLabel();
    nameLabel->setText(name);
    QFont font("微软雅黑", 10);
    nameLabel->setFont(font);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet(
        "QLabel {"
        " color: rgb(80, 80, 80);"
        " margin-top: 5px;"
        "}");

    // 截断名字
    static const int MAX_WIDTH = 45;
    QFontMetrics metrics(font);
    int totalWidth = metrics.horizontalAdvance(name);
    if (totalWidth > MAX_WIDTH)
    {
        static const QString tail = "...";
        int tailWidth = metrics.horizontalAdvance(tail);
        int availableWidth = MAX_WIDTH - tailWidth;
        int availableSize = name.size() * ((double)availableWidth / totalWidth);
        QString subName = name.left(availableSize);
        nameLabel->setText(subName + tail);
    }

    layout->addWidget(avatarBtn);
    layout->addWidget(nameLabel);
}

QPushButton *AvatarItem::getAvatar()
{
    return avatarBtn;
}

SessionDetailWidget::SessionDetailWidget(QWidget *parent, const UserInfo &userInfo)
    : QDialog(parent)
    , userInfo(userInfo)
{
    // 基本属性
    setWindowTitle("会话详情");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setFixedSize(300, 200);
    setStyleSheet(
        "QDialog {"
        " background-color: rgb(245, 245, 245);"
        "}"
        "QPushButton {"
        " border: none;"
        " outline: none;"
        "}");
    setAttribute(Qt::WA_DeleteOnClose);

    // 垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    setLayout(mainLayout);

    // 用户头像区域水平布局
    QHBoxLayout* userLayout = new QHBoxLayout();
    userLayout->setSpacing(20);
    userLayout->setContentsMargins(0, 0, 0, 0);

    // 创建群聊按钮
    AvatarItem* createGroupBtn = new AvatarItem(QIcon(":/resource/image/cross.png"), "添加");
    userLayout->addWidget(createGroupBtn, 0, Qt::AlignCenter);

// 当前用户的信息
#if TEST_UI
    AvatarItem* currentUser = new AvatarItem(QIcon(":/resource/image/defaultAvatar.png"), "张三");
    userLayout->addWidget(currentUser, 0, Qt::AlignCenter);
#else
    AvatarItem* currentUser = new AvatarItem(userInfo.avatar, userInfo.nickname);
    userLayout->addWidget(currentUser, 0, Qt::AlignCenter);
#endif

    mainLayout->addLayout(userLayout);

    // 删除好友按钮
    deleteFriendBtn = new QPushButton("删除好友");
    deleteFriendBtn->setFixedHeight(40);
    deleteFriendBtn->setStyleSheet(
        "QPushButton {"
        " border: 1px solid rgb(240, 150, 150);"
        " border-radius: 5px;"
        " background-color: rgb(250, 230, 230);"
        " color: rgb(180, 80, 80);"
        " font-size: 14px;"
        "}"
        "QPushButton:hover {"
        " background-color: rgb(250, 210, 210);"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(240, 150, 150);"
        "}");
    mainLayout->addWidget(deleteFriendBtn);

    // 信号槽连接
    connect(createGroupBtn->getAvatar(), &QPushButton::clicked, this, [=]() {
        ChooseFriendDialog* chooseFriendDialog = new ChooseFriendDialog(this, { userInfo.userId }, true);
        chooseFriendDialog->exec();
    });

    connect(deleteFriendBtn, &QPushButton::clicked, this, &SessionDetailWidget::clickDeleteFriendBtn);
}

void SessionDetailWidget::clickDeleteFriendBtn()
{
    // 弹出确认删除对话框
    auto result = QMessageBox::warning(this, "确认删除", "确认删除该好友?", QMessageBox::Ok | QMessageBox::Cancel);
    if (result != QMessageBox::Ok)
    {
        LOG() << "用户取消了好友删除";
        return;
    }

    // 发送删除好友请求
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    dataCenter->deleteFriendAsync(userInfo.userId);
    close();
}
