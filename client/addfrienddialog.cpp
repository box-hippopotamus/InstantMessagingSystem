#include "addfrienddialog.h"

using namespace IM::Model;

FriendResultItem::FriendResultItem(const UserInfo &userInfo)
    : userInfo(userInfo)
{
    // 基本属性
    setFixedHeight(60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(8);
    layout->setContentsMargins(12, 0, 12, 0);
    setLayout(layout);

    // 头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(44, 44);
    avatarBtn->setIconSize(QSize(44, 44));
    avatarBtn->setIcon(userInfo.avatar);
    avatarBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    border-radius: 4px;"
        "}");

    // 昵称
    QLabel* nameLabel = new QLabel();
    nameLabel->setFixedHeight(30);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 15px;"
        "    font-weight: 600;"
        "    color: rgb(80, 80, 80);"
        "}");
    nameLabel->setText(userInfo.nickname);

    // 个性签名
    QLabel* descLabel = new QLabel();
    descLabel->setFixedHeight(30);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    descLabel->setStyleSheet(
         "QLabel {"
        "    font-size: 13px;"
        "    color: rgb(120, 120, 120);"
        "}");
    descLabel->setText(userInfo.description);

    // 添加好友按钮
    addBtn = new QPushButton();
    addBtn->setFixedSize(90, 32);
    addBtn->setText("添加好友");
    static const QString btnStyle =
        "QPushButton {"
        "    border: 1px solid rgb(208, 221, 247);"
        "    background-color: rgb(240, 243, 250);"
        "    color: rgb(100, 130, 180);"
        "    border-radius: 5px;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(230, 238, 255);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(208, 221, 247);"
        "}";
    addBtn->setStyleSheet(btnStyle);

    // 布局管理器
    layout->addWidget(avatarBtn, 0, 0, 2, 1);
    layout->addWidget(nameLabel, 0, 1);
    layout->addWidget(descLabel, 1, 1);
    layout->addWidget(addBtn, 0, 2, 2, 1);

    // 连接信号槽
    connect(addBtn, &QPushButton::clicked, this, &FriendResultItem::clickAddBtn);
}

void FriendResultItem::clickAddBtn()
{
    // 发送好友申请
    DataCenter* dataCenter = DataCenter::getInstance();
    dataCenter->addFriendApplyAsync(userInfo.userId);

    // 禁用按钮
    addBtn->setEnabled(false);
    addBtn->setText("已申请");
    addBtn->setStyleSheet("QPushButton { "
                          "    border: 1px solid rgb(208, 221, 247);"
                          "    border-radius: 5px;"
                          "    font-size: 13px;"
                          "    color: rgb(255, 255, 255); "
                          "    background-color: rgb(200, 200, 200); "
                          "}");
}

AddFriendDialog::AddFriendDialog(QWidget *parent)
    : QDialog(parent)
{
    // 基本属性
    setFixedSize(460, 460);
    setWindowTitle("添加好友");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setStyleSheet("QDialog { background-color: rgb(245, 245, 245); }");
    setAttribute(Qt::WA_DeleteOnClose);

    // 布局管理器
    layout = new QGridLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(15, 15, 15, 15);
    setLayout(layout);

    // 搜索框
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(42);
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    static const QString style =
        "QLineEdit {"
        "    border: 1px solid rgb(230, 230, 230);"
        "    border-radius: 5px;"
        "    font-size: 14px;"
        "    background-color: rgb(255, 255, 255);"
        "    padding-left: 8px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid rgb(208, 221, 247);"
        "}";
    searchEdit->setStyleSheet(style);
    searchEdit->setPlaceholderText("手机号/用户序号/昵称");
    layout->addWidget(searchEdit, 0, 0, 1, 8);

    // 搜索按钮
    QPushButton* searchBtn = new QPushButton();
    searchBtn->setFixedSize(42, 42);
    searchBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    searchBtn->setIconSize(QSize(24, 24));
    searchBtn->setIcon(QIcon(":/resource/image/search.png"));
    static const QString btnStyle =
        "QPushButton {"
        "    border: 1px solid rgb(230, 230, 230);"
        "    background-color: rgb(255, 255, 255);"
        "    border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(240, 243, 250);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(208, 221, 247);"
        "}";
    searchBtn->setStyleSheet(btnStyle);
    layout->addWidget(searchBtn, 0, 8, 1, 1);

    initResultArea(); // 初始化滚动区域

#if TEST_UI
    QIcon avatar(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 20; ++i)
    {
        UserInfo* userInfo = new UserInfo();
        userInfo->userId = QString::number(1000 + i);
        userInfo->nickname = "张三" + QString::number(i);
        userInfo->description = "这是一段个性签名";
        userInfo->avatar = avatar;
        addResult(*userInfo);
    }
#endif

    // 连接信号槽
    connect(searchBtn, &QPushButton::clicked, this, &AddFriendDialog::clickSearchBtn);
}

void AddFriendDialog::initResultArea()
{
    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal {height: 0;} ");
    scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "    width: 5px;"
        "    background: rgb(240, 243, 250);"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgb(208, 221, 247);"
        "    min-height: 20px;"
        "    border-radius: 2px;"
        "}");
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: 1px solid rgb(230, 230, 230);"
        "    border-radius: 5px;"
        "    background-color: rgb(255, 255, 255);"
        "}");
    layout->addWidget(scrollArea, 1, 0, 1, 9);

    // QWidget
    resultContainer = new QWidget();
    resultContainer->setObjectName("resultContainer");
    resultContainer->setStyleSheet(
        "#resultContainer {"
        "    background-color: rgb(255, 255, 255);"
        "}");
    scrollArea->setWidget(resultContainer);

    // 布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    resultContainer->setLayout(vlayout);
}

void AddFriendDialog::addResult(const UserInfo &userInfo)
{
    FriendResultItem* item = new FriendResultItem(userInfo);
    resultContainer->layout()->addWidget(item);
}

void AddFriendDialog::clear()
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(resultContainer->layout());
    for (int i = layout->count() - 1; i >= 0; --i)
    {
        QLayoutItem* layoutItem = layout->takeAt(i);
        if (layoutItem == nullptr || layoutItem->widget() == nullptr)
            continue;

        layoutItem->widget()->deleteLater();
    }
}

void AddFriendDialog::setSearchKey(const QString searchKey)
{
    searchEdit->setText(searchKey);
}

void AddFriendDialog::clickSearchBtn()
{
    // 拿到输入框的内容
    const QString& text = searchEdit->text();
    if (text.isEmpty())
        return;

    // 发起请求
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::searchUserDone, this, &AddFriendDialog::clickSearchBtnDone, Qt::UniqueConnection);
    dataCenter->searchUserAsync(text);
}

void AddFriendDialog::clickSearchBtnDone()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<UserInfo>* searchResult = dataCenter->getSearchUserResult();
    if (searchResult == nullptr)
        return;

    this->clear();
    for (const auto& u : *searchResult)
        addResult(u);
}
