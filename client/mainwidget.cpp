#include "mainwidget.h"

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
{   
    setWindowTitle("Instant Message System");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    resize(1000, 700);  // 窗口初始大小

    // 初始化窗口
    initMainWindow();
    initLeftWindow();
    initRightWindow();
    initMidWindow();
    initSignalSlot();
    initWebsocket();
}

MainWidget::~MainWidget() {}

MainWidget* MainWidget::getInstance()
{
    if (instance == nullptr)
        instance = new MainWidget();

    return instance;
}

// 初始化主窗口
void MainWidget::initMainWindow()
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(0); // 内部元素之间的间隔
    layout->setContentsMargins(0, 0, 0, 0); //元素距离四个边界的距离
    setLayout(layout);

    windowLeft = new QWidget();
    windowMid = new QWidget();
    windowRight = new QWidget();

    // widget 宽度
    windowLeft->setFixedWidth(60);
    windowMid->setFixedWidth(250);
    windowRight->setMinimumWidth(500);

    // widget 背景色
    windowLeft->setStyleSheet("QWidget { background-color: rgb(208, 221, 247); }");
    windowMid->setStyleSheet("QWidget { background-color: rgb(240, 243, 250); }");
    windowRight->setStyleSheet(
        "QWidget {"
        "    background-color: rgb(245, 245, 245);"
        "}"
        );

    // 添加到主窗口
    layout->addWidget(windowLeft);
    layout->addWidget(windowMid);
    layout->addWidget(windowRight);
}

// 初始化选项栏
void MainWidget::initLeftWindow()
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(20);
    layout->setContentsMargins(0, 10, 0, 0);
    windowLeft->setLayout(layout);

    // 按钮样式
    static const QString buttonStyle =
                    "QPushButton {"
                    "    background-color: transparent;"
                    "    border: none;"
                    "}";

    // 用户头像
    userAvatar = new QPushButton();
    userAvatar->setFixedSize(45, 45);
    userAvatar->setIconSize(QSize(45, 45));
    // userAvatar->setIcon(QIcon(":/resource/image/defaultAvatar.png"));
    userAvatar->setStyleSheet(buttonStyle);
    userAvatar->setCursor(Qt::PointingHandCursor);
    layout->addWidget(userAvatar, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 会话添加标签页按钮
    sessionTabBtn = new QPushButton();
    sessionTabBtn->setFixedSize(25, 25);
    sessionTabBtn->setIconSize(QSize(25, 25));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_active.png"));
    sessionTabBtn->setStyleSheet(buttonStyle);
    sessionTabBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(sessionTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 好友添加标签页按钮
    friendTabBtn = new QPushButton();
    friendTabBtn->setFixedSize(25, 25);
    friendTabBtn->setIconSize(QSize(25, 25));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    friendTabBtn->setStyleSheet(buttonStyle);
    friendTabBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(friendTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 好友申请标签页按钮
    applyTabBtn = new QPushButton();
    applyTabBtn->setFixedSize(25, 25);
    applyTabBtn->setIconSize(QSize(25, 25));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    applyTabBtn->setStyleSheet(buttonStyle);
    applyTabBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(applyTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);
    layout->addStretch(20);
}

// 初始化会话栏
void MainWidget::initMidWindow()
{
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(0, 20, 0, 0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(10);
    windowMid->setLayout(layout);

    // 颜色变量
    const QString lightGray = "rgb(226, 226, 226)";
    const QString pressedGray = "rgb(209, 209, 209)";
    const QString placeholderGray = "rgb(150, 150, 150)";

    // 搜索框样式
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(30);
    searchEdit->setPlaceholderText("搜索");
    searchEdit->setStyleSheet(QString(
                                  "QLineEdit {"
                                  "   border-radius: 5px;"
                                  "   background-color: %1;"
                                  "   padding: 0 10px;"
                                  "   font-size: 14px;"
                                  "   border: 1px solid transparent;"
                                  "}"
                                  "QLineEdit:focus {"
                                  "   border: 1px solid rgb(208, 221, 247);"
                                  "}"
                                  "QLineEdit::placeholder {"
                                  "   color: %2;"
                                  "}"
                                  ).arg(lightGray, placeholderGray));

    // 添加好友按钮样式
    addFriendBtn = new QPushButton();
    addFriendBtn->setFixedSize(30, 30);
    addFriendBtn->setIcon(QIcon(":/resource/image/cross.png"));
    addFriendBtn->setIconSize(QSize(16, 16));
    addFriendBtn->setCursor(Qt::PointingHandCursor);

    static const QString btnStyle = QString(
                           "QPushButton {"
                           "   border-radius: 5px;"
                           "   background-color: %1;"
                           "   border: none;"
                           "}"
                           "QPushButton:hover {"
                           "   background-color: %2;"
                           "}"
                           "QPushButton:pressed {"
                           "   background-color: %3;"
                           "}"
                           ).arg(lightGray, pressedGray, "rgb(190, 190, 190)");

    addFriendBtn->setStyleSheet(btnStyle);

    sessionFriendArea = new SessionFriendArea();

    // QSpacerItem 插入空隙
    layout->addItem(new QSpacerItem(10, 0), 0, 0);
    layout->addWidget(searchEdit, 0, 1);
    layout->addItem(new QSpacerItem(10, 0), 0, 2);
    layout->addWidget(addFriendBtn, 0, 3);
    layout->addItem(new QSpacerItem(10, 0), 0, 4);
    layout->addWidget(sessionFriendArea, 1, 0, 1, 5);
}

// 初始化消息栏
void MainWidget::initRightWindow()
{
    // 右侧窗口布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    windowRight->setLayout(vlayout);

    // 上方标题栏
    QWidget* titleWidget = new QWidget();
    titleWidget->setFixedHeight(60);
    titleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleWidget->setObjectName("titleWidget");
    titleWidget->setStyleSheet(
        "#titleWidget {"
        "    border-bottom: 1px solid rgb(225, 228, 235);"
        "    background-color: rgb(248, 249, 252);"
        "}"
        );
    vlayout->addWidget(titleWidget);

    // 标题栏内部布局
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(12);
    hlayout->setContentsMargins(20, 0, 20, 0);
    titleWidget->setLayout(hlayout);

    // 会话标题标签
    sessionTitleLabel = new QLabel();
    sessionTitleLabel->setStyleSheet(
        "QLabel {"
        "    border-bottom: 1px solid rgb(225, 228, 235);"
        "    background-color: rgb(248, 249, 252);"
        "    font-size: 18px;"
        "    font-weight: 600;"
        "    color: rgb(60, 60, 60);"
        "    padding: 5px 0;"
        "}"
        );
    hlayout->addWidget(sessionTitleLabel, 1);

    // 额外功能按钮
    extraBtn = new QPushButton();
    extraBtn->setFixedSize(36, 36);
    extraBtn->setIconSize(QSize(24, 24));
    extraBtn->setIcon(QIcon(":/resource/image/more.png"));
    extraBtn->setCursor(Qt::PointingHandCursor);
    extraBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    border-radius: 18px;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(200, 210, 230, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(180, 190, 210, 80);"
        "}"
        );
    hlayout->addWidget(extraBtn, 0, Qt::AlignRight);

    // 消息展示区
    messageShowArea = new MessageShowArea();
    vlayout->addWidget(messageShowArea, 1);

    // 消息编辑区
    messageEditArea = new MessageEditArea();
    vlayout->addWidget(messageEditArea, 0, Qt::AlignBottom);

#if TEST_UI
    sessionTitleLabel->setText("测试会话标题");
#endif
}

// 初始化信号槽
void MainWidget::initSignalSlot()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();

    // 标签页按钮切换
    connect(sessionTabBtn, &QPushButton::clicked, this, &MainWidget::switchTabToSession);
    connect(friendTabBtn, &QPushButton::clicked, this, &MainWidget::switchTabToFriend);
    connect(applyTabBtn, &QPushButton::clicked, this, &MainWidget::switchTabToApply);

    // 点击头像
    connect(userAvatar, &QPushButton::clicked, this, [this](){
        SelfInfoWidget* selfInfoWidget = new SelfInfoWidget(this);
        selfInfoWidget->exec();
    });

    // 点击会话详情
    connect(extraBtn, &QPushButton::clicked, this, [=]() {
        IM::Model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(dataCenter->getCurrentChatSessionId());
        if (chatSessionInfo == nullptr)
        {
            LOG() << "当前会话不存在, 无法弹出会话详情对话框";
            return;
        }

        if (chatSessionInfo->userId != "")
        {
            UserInfo* userInfo = dataCenter->findFriendById(chatSessionInfo->userId);
            if (userInfo == nullptr)
            {
                LOG() << "单聊会话对应的用户不存在, 无法弹出会话详情窗口";
                return;
            }
            SessionDetailWidget* sessionDetailWidget = new SessionDetailWidget(this, *userInfo);
            sessionDetailWidget->exec();
        }
        else
        {
            GroupSessionDetailWidget* groupSessionDetailWidget = new GroupSessionDetailWidget(this);
            groupSessionDetailWidget->exec();
        }
    });

    // 添加好友窗口
    connect(addFriendBtn, &QPushButton::clicked, this, [this](){
        AddFriendDialog* addFriendDialog = new AddFriendDialog(this);
        addFriendDialog->exec();
    });

    // 搜索框
    connect(searchEdit, &QLineEdit::editingFinished, this, [this](){
        AddFriendDialog* addFriendDialog = new AddFriendDialog(this);
        addFriendDialog->setSearchKey(searchEdit->text());
        searchEdit->clear();  // 清空搜索框
        addFriendDialog->exec();
    });

    // 获取网络数据
    connect(dataCenter, &IM::Model::DataCenter::getMyselfDone, this, [=]() {
        // 从 DataCenter 中拿到响应结果的 myself, 把里面的头像取出来, 显示到界面上.
        auto myself = dataCenter->getMyself();
        userAvatar->setIcon(myself->avatar);
    });

    dataCenter->getMyselfAsync();

    loadFriendList();  // 获取好友列表
    loadSessionList(); // 获取会话列表
    loadApplyList();   // 获取好友申请列表

    connect(dataCenter, &IM::Model::DataCenter::leaveGroupDone, this, [=](const QString &chatSessionId) {
        // 刷新左侧列表会话
        updateChatSessionList();

        // 清空消息区域
        if (dataCenter->getCurrentChatSessionId() == chatSessionId)
        {
            messageShowArea->clear();
            dataCenter->setCurrentChatSessionId("");
        }
    });

    // 处理修改头像
    connect(dataCenter, &IM::Model::DataCenter::changeAvatarDone, this, [=]() {
        UserInfo* myself = dataCenter->getMyself();
        userAvatar->setIcon(myself->avatar);
    });

    // 处理删除好友
    connect(dataCenter, &IM::Model::DataCenter::deleteFriendDone, this, [=]() {
        // 更新会话列表和好友列表
        updateFriendList();
        updateChatSessionList();
        LOG() << "删除好友完成";
    });

    connect(dataCenter, &IM::Model::DataCenter::clearCurrentSession, this, [=]() {
        sessionTitleLabel->setText("");
        messageShowArea->clear();
        dataCenter->setCurrentChatSessionId("");
        LOG() << "清空当前会话完成";
    });

    // 处理添加好友申请
    connect(dataCenter, &IM::Model::DataCenter::addFriendApplyDone, this, [=]() {
        Toast::showMessage("好友申请已发送");
    });

    // 处理添加好友申请推送数据
    connect(dataCenter, &IM::Model::DataCenter::receiveFriendApplyDone, this, [=]() {
        updateApplyList();
        Toast::showMessage("收到新的好友申请");
    });

    // 处理同意好友申请
    connect(dataCenter, &IM::Model::DataCenter::acceptFriendApplyDone, this, [=]() {
        updateApplyList();
        updateFriendList();
        Toast::showMessage("好友已经添加完成");
    });

    // 处理拒绝好友申请
    connect(dataCenter, &IM::Model::DataCenter::rejectFriendApplyDone, this, [=]() {
        // 需要更新好友申请列表. 刚才拒绝的这一项, 是需要删除掉的.
        updateApplyList();
        Toast::showMessage("好友申请已经拒绝");
    });

    // 处理好友申请结果的推送数据
    connect(dataCenter, &IM::Model::DataCenter::receiveFriendProcessDone, this, [=](const QString& nickname, bool agree) {
        if (agree)
        {
            updateFriendList();
            Toast::showMessage(nickname + " 已经同意了你的好友申请");
        }
        else
        {
            Toast::showMessage(nickname + " 已经拒绝了你的好友申请");
        }
    });

    // 处理创建群聊的响应信号
    connect(dataCenter, &IM::Model::DataCenter::createGroupChatSessionDone, this, [=]() {
        Toast::showMessage("创建群聊会话请求已经发送!");
    });

    // 处理修改群名响应
    connect(dataCenter, &IM::Model::DataCenter::groupModifyNameDone, this,
            [=](const QString& chatSessionId, const QString& groupName){
        // 刷新左侧列表会话
        updateChatSessionList();

        // 修改标题
        if (dataCenter->getCurrentChatSessionId() == chatSessionId)
            sessionTitleLabel->setText(groupName);
    });

    // 处理创建会话的推送数据
    connect(dataCenter, &IM::Model::DataCenter::receiveSessionCreateDone, this, [=]() {
        updateChatSessionList();
        Toast::showMessage("群聊拉取成功!");
    });
}

void MainWidget::initWebsocket()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    dataCenter->initWebsocket();
}

// 切换到会话
void MainWidget::switchTabToSession()
{
    activeTab = ActiveTab::SESSION_LIST;

    // 调整图标
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_active.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));

    // 加载数据
    loadSessionList();
}

// 切换到好友
void MainWidget::switchTabToFriend()
{
    activeTab = ActiveTab::FRIEND_LIST;

    // 调整图标
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_active.png"));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_inactive.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));

    // 加载数据
    loadFriendList();
}

// 切换到申请
void MainWidget::switchTabToApply()
{
    activeTab = ActiveTab::APPLY_LIST;

    // 调整图标
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_active.png"));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_inactive.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));

    // 加载数据
    loadApplyList();
}

// 加载会话列表
void MainWidget::loadSessionList()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getChatSessionList() != nullptr)
    {
        updateChatSessionList();
    }
    else
    {
        connect(dataCenter, &IM::Model::DataCenter::getChatSessionListDone, this, &MainWidget::updateChatSessionList, Qt::UniqueConnection);
        dataCenter->getChatSessionListAsync();
    }
}

// 加载好友列表
void MainWidget::loadFriendList()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getFriendList() != nullptr)
    {
        // 从内存这个列表中加载数据
        updateFriendList();
    }
    else
    {
        // 通过网络来加载数据
        connect(dataCenter, &IM::Model::DataCenter::getFriendListDone, this, &MainWidget::updateFriendList, Qt::UniqueConnection);
        dataCenter->getFriendListAsync();
    }
}

// 加载好友申请列表
void MainWidget::loadApplyList()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getApplyList() != nullptr)
    {
        updateApplyList();
    }
    else
    {
        connect(dataCenter, &IM::Model::DataCenter::getApplyListDone, this, &MainWidget::updateApplyList, Qt::UniqueConnection);
        dataCenter->getApplyListAsync();
    }
}

void MainWidget::updateFriendList()
{
    if (activeTab != ActiveTab::FRIEND_LIST)
        return;

    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<UserInfo>* friendList = dataCenter->getFriendList();

    // 清空一下之前界面上的数据
    sessionFriendArea->clear();

    // 遍历好友列表, 添加到界面上
    for (const auto& f : *friendList)
        sessionFriendArea->addItem(ItemType::FriendItemType, f.userId, f.avatar, f.nickname, f.description);
}

void MainWidget::updateChatSessionList()
{
    if (activeTab != ActiveTab::SESSION_LIST)
        return;

    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<IM::Model::ChatSessionInfo>* chatSessionList = dataCenter->getChatSessionList();
    sessionFriendArea->clear();

    for (const auto& c : *chatSessionList)
    {
        switch (c.lastMessage.messageType)
        {
        case IM::Model::MessageType::TEXT_TYPE:
            sessionFriendArea->addItem(ItemType::SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, c.lastMessage.content);
            break;
        case IM::Model::MessageType::IMAGE_TYPE:
            sessionFriendArea->addItem(ItemType::SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "[图片]");
            break;
        case IM::Model::MessageType::FILE_TYPE:
            sessionFriendArea->addItem(ItemType::SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "[文件]");
            break;
        case IM::Model::MessageType::SPEECH_TYPE:
            sessionFriendArea->addItem(ItemType::SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, "[语音]");
            break;
        default:
            LOG() << "错误的消息类型! messageType=" << (int)c.lastMessage.messageType;
        }
    }
}

void MainWidget::updateApplyList()
{
    if (activeTab != ActiveTab::APPLY_LIST)
        return;

    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<UserInfo>* applyList = dataCenter->getApplyList();

    sessionFriendArea->clear();

    for (const auto& u : *applyList)
        sessionFriendArea->addItem(ItemType::ApplyItemType, u.userId, u.avatar, u.nickname, "");
}

void MainWidget::loadRecentMessage(const QString &chatSessionId)
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();

    if (dataCenter->getRecentMessageList(chatSessionId) != nullptr)
    {
        updateRecentMessage(chatSessionId);
    }
    else
    {
        connect(dataCenter, &IM::Model::DataCenter::getRecentMessageListDone, this, &MainWidget::updateRecentMessage, Qt::UniqueConnection);
        dataCenter->getRecentMessageListAsync(chatSessionId, true);
    }
}

void MainWidget::updateRecentMessage(const QString &chatSessionId)
{
    // 该会话的最近消息列表
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    auto* recentMessageList = dataCenter->getRecentMessageList(chatSessionId);

    // 清空原有界面上显示的消息列表
    messageShowArea->clear();

    // 消息列表显示到界面上
    for (int i = recentMessageList->size() - 1; i >= 0; --i)
    {
        const Message& message = recentMessageList->at(i);
        bool isLeft = message.sender.userId != dataCenter->getMyself()->userId;
        messageShowArea->addFrontMessage(isLeft, message);
    }

    // 设置会话标题
    IM::Model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(chatSessionId);
    if (chatSessionInfo != nullptr)// 把会话名称显示到界面上.
        sessionTitleLabel->setText(chatSessionInfo->chatSessionName);

    // 保存当前选中的会话
    dataCenter->setCurrentChatSessionId(chatSessionId);
    //滚动条滚动到末尾
    messageShowArea->scrollToEnd();
}

void MainWidget::switchSession(const QString &userId)
{
    // 先找到对应的会话元素
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    IM::Model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionByUserId(userId);
    if (chatSessionInfo == nullptr)
    {
        LOG() << "[严重错误] 当前选中的好友, 对应的会话不存在!";
        return;
    }

    // 会话置顶
    dataCenter->topChatSessionInfo(*chatSessionInfo);

    // 切换到会话列表标签页
    switchTabToSession();

    // 加载这个会话历史消息
    sessionFriendArea->clickItem(0);
}

MessageShowArea *MainWidget::getMessageShowArea()
{
    return messageShowArea;
}
