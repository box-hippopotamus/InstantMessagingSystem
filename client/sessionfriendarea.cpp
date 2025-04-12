#include "sessionfriendarea.h"
#include "mainwidget.h"

SessionFriendArea::SessionFriendArea(QScrollArea *parent)
    : QScrollArea{parent}
{
    setWidgetResizable(true); // 开启滚动效果

    // 滚动条样式
    static const QString scrollBarStyle =
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: rgba(240, 243, 250, 50);"
        "    width: 8px;"
        "    margin: 2px 0 2px 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(180, 190, 210, 150);"
        "    min-height: 30px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(150, 160, 180, 200);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    height: 0px;"
        "    background: none;"
        "}"
        "QScrollBar:horizontal { height: 0px; }";

    verticalScrollBar()->setStyleSheet(scrollBarStyle);
    horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");

    setStyleSheet("QWidget { border: none;}");

    // 创建 widget
    container = new QWidget();
    container->setFixedWidth(310);
    setWidget(container);

    // widget 布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);
    container->setLayout(layout);

#if TEST_UI
    QIcon icon(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 30; ++i)
    {
        addItem(ItemType::SessionItemType, QString::number(i), icon, "张三" + QString::number(i), "最后一条消息" + QString::number(i));
    }
#endif
}

void SessionFriendArea::clear()
{
    QLayout* layout = container->layout();

    // 遍历布局管理器中的所有元素, 并依次从布局管理器中删除掉
    for (int i = layout->count() - 1; i >= 0; --i)
    {
        QLayoutItem* item = layout->takeAt(i);
        if (item->widget())
            delete item->widget();
    }
}

void SessionFriendArea::addItem(ItemType itemType, const QString& id, const QIcon &avatar, const QString &name, const QString &text)
{
    SessionFriendItem* item = nullptr;

    switch(itemType)
    {
    case ItemType::SessionItemType:
        item = new SessionItem(this, id, avatar, name, text);
        break;
    case ItemType::FriendItemType:
        item = new FriendItem(this, id, avatar, name, text);
        break;
    case ItemType::ApplyItemType:
        item = new ApplyItem(this, id, avatar, name);
        break;
    default:
        LOG() << "错误的 ItemType! itemType=" << (int)itemType;
        return;
    }

    container->layout()->addWidget(item);
}

void SessionFriendArea::clickItem(int index)
{
    if (index < 0 || index >= container->layout()->count())
    {
        LOG() << "点击元素的下标超出范围! index=" << index;
        return;
    }

    QLayoutItem* layoutItem = container->layout()->itemAt(index);
    if (layoutItem == nullptr || layoutItem->widget() == nullptr)
    {
        LOG() << "指定的元素不存在! index=" << index;
        return;
    }

    SessionFriendItem* item = dynamic_cast<SessionFriendItem*>(layoutItem->widget());
    item->select();
}

// ===============================================
// ============== SessionFriendItem ==============
// ===============================================
SessionFriendItem::SessionFriendItem(QWidget *owner,
                                     const QIcon &avatar,
                                     const QString &name,
                                     const QString &text)
    : owner(owner)
{
    setFixedHeight(70);
    setStyleSheet("QWidget { border-radius: 4px; }");

    // 创建网格布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(15, 5, 10, 5);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(0);
    setLayout(layout);

    // 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(46, 46);
    avatarBtn->setIconSize(QSize(42, 42));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet(
        "QPushButton {"
        "border: 1px solid rgba(200, 205, 220, 0.8);"
        "border-radius: 5px;"
        "background-color: transparent;"
        "}");

    // 创建名字
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(name);
    nameLabel->setStyleSheet(
        "QLabel {"
        "font-size: 15px;"
        "font-weight: 600;"
        "color: rgb(50, 50, 50);"
        "}");
    nameLabel->setFixedHeight(28);

    // 创建消息预览的label
    messageLabel = new QLabel();
    messageLabel->setText(text);
    messageLabel->setStyleSheet(
        "QLabel {"
        "font-size: 13px;"
        "color: rgb(150, 150, 150);"
        "}");
    messageLabel->setFixedHeight(24);

    layout->addWidget(avatarBtn, 0, 0, 2, 2);
    layout->addWidget(nameLabel, 0, 2, 1, 10);
    layout->addWidget(messageLabel, 1, 2, 1, 10);
}

void SessionFriendItem::paintEvent(QPaintEvent *event)
{
    (void) event;
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SessionFriendItem::mousePressEvent(QMouseEvent *event)
{
    (void) event;
    select();
}

void SessionFriendItem::enterEvent(QEnterEvent *event)
{
    (void) event;

    //  item 是选中状态, 则背景色不受到该逻辑影响
    if (selected)
        return;

    setStyleSheet("QWidget { background-color: rgb(232, 240, 255);}");
}

void SessionFriendItem::leaveEvent(QEvent *event)
{
    (void) event;

    // item 是选中状态, 则背景色不受到该逻辑影响
    if (selected)
        return;

    // 还原背景色
    setStyleSheet("QWidget { background-color: rgb(240, 243, 250);}");
}

void SessionFriendItem::select()
{
    // 拿到所有的兄弟元素, 还原其他元素的背景色
    const QObjectList children = parentWidget()->children();
    for (QObject* child : children)
    {
        if (!child->isWidgetType()) // 判定是否是 widget.
            continue;

        // child 强转成 SessionFriendItem
        SessionFriendItem* item = dynamic_cast<SessionFriendItem*>(child);
        if (item->selected)
        {
            item->selected = false;
            item->setStyleSheet("QWidget { background-color: rgb(240, 243, 250); }");
        }
    }

    // 设置当前 item 背景色,
    setStyleSheet("QWidget { background-color: rgb(225, 235, 255); }");
    selected = true;

    // 调用 active
    active();
}

void SessionFriendItem::active()
{
    // 父类的 active,不实现任何逻辑.
}

// =========================================
// ============== SessionItem ==============
// =========================================
SessionItem::SessionItem(QWidget *owner,
                         const QString &chatSessionId,
                         const QIcon &avatar,
                         const QString &name,
                         const QString &lastMessage)
    : SessionFriendItem(owner, avatar, name, lastMessage)
    , chatSessionId(chatSessionId)
    , text(lastMessage)
{
    // 处理更新最后一个消息的信号
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::updateLastMessage, this, &SessionItem::updateLastMessage);

    // 需要显示出未读消息的数目, 为了支持客户端重启之后, 未读消息仍然能正确显示.
    int unread = dataCenter->getUnread(chatSessionId);
    if (unread > 0) // 存在未读消息
        messageLabel->setText(QString("[未读%1条] ").arg(unread) + text);
}

void SessionItem::active()
{
    // 加载会话历史消息
    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->loadRecentMessage(chatSessionId);

    // 清空未读消息的数据, 并且更新显示
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    dataCenter->clearUnread(chatSessionId);

    // 更新界面的显示. 把会话消息预览这里,删掉 "[未读x条]"
    messageLabel->setText(text);
}

void SessionItem::updateLastMessage(const QString &chatSessionId)
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (this->chatSessionId != chatSessionId)
        return;

    // 获取最后一条消息
    QList<Message>* messageList = dataCenter->getRecentMessageList(chatSessionId);
    if (messageList == nullptr || messageList->size() == 0)
        return;

    const Message& lastMessage = messageList->back();

    // 显示文本内容
    switch(lastMessage.messageType)
    {
    case IM::Model::MessageType::TEXT_TYPE:
        text = lastMessage.content;
        break;
    case IM::Model::MessageType::IMAGE_TYPE:
        text = "[图片]";
        break;
    case IM::Model::MessageType::FILE_TYPE:
        text = "[文件]";
        break;
    case IM::Model::MessageType::SPEECH_TYPE:
        text = "[语音]";
        break;
    default:
        LOG() << "错误的消息类型!";
        return;
    }

    if (chatSessionId == dataCenter->getCurrentChatSessionId())
    {
        messageLabel->setText(text);
    }
    else
    {
        int unread = dataCenter->getUnread(chatSessionId);
        if (unread > 0)
            messageLabel->setText(QString("[未读%1条] ").arg(unread) + text);
    }
}

// ========================================
// ============== FriendItem ==============
// ========================================
FriendItem::FriendItem(QWidget *owner,
                       const QString &userId,
                       const QIcon &avatar,
                       const QString &name,
                       const QString &description)
    : SessionFriendItem(owner, avatar, name, description)
    , userId(userId)
{}

void FriendItem::active()
{
    // 点击之后, 激活对应的会话列表元素
    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->switchSession(userId);
}


// =======================================
// ============== ApplyItem ==============
// =======================================
ApplyItem::ApplyItem(QWidget *owner,
                     const QString &userId,
                     const QIcon &avatar,
                     const QString &name)
    : SessionFriendItem(owner, avatar, name, "")
    , userId(userId)
{
    // 移除父类 messageLabel
    QGridLayout* layout = dynamic_cast<QGridLayout*>(this->layout());
    layout->removeWidget(messageLabel);
    delete messageLabel;

    // 设置按钮QSS样式
    static const QString buttonStyle =
        "QPushButton {"
        "    background-color: rgb(230, 240, 255);"
        "    border: 1px solid rgb(200, 210, 230);"
        "    border-radius: 4px;"
        "    padding: 5px 10px;"
        "    color: rgb(50, 50, 50);"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(220, 230, 250);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(210, 220, 245);"
        "}";

    // 创建按钮
    QPushButton* acceptBtn = new QPushButton();
    acceptBtn->setText("同意");
    acceptBtn->setCursor(Qt::PointingHandCursor);
    acceptBtn->setStyleSheet(buttonStyle);

    QPushButton* rejectBtn = new QPushButton();
    rejectBtn->setText("拒绝");
    rejectBtn->setCursor(Qt::PointingHandCursor);
    rejectBtn->setStyleSheet(buttonStyle);

    // 添加到布局管理器
    layout->addWidget(acceptBtn, 1, 2, 1, 1);
    layout->addWidget(rejectBtn, 1, 4, 1, 1);

    // 添加信号槽
    connect(acceptBtn, &QPushButton::clicked, this, &ApplyItem::acceptFriendApply);
    connect(rejectBtn, &QPushButton::clicked, this, &ApplyItem::rejectFriendApply);
}

void ApplyItem::active()
{}

void ApplyItem::acceptFriendApply()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    dataCenter->acceptFriendApplyAsync(userId);
}

void ApplyItem::rejectFriendApply()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    dataCenter->rejectFriendApplyAsync(userId);
}

