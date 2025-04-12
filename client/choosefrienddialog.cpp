#include "choosefrienddialog.h"

ChooseFriendItem::ChooseFriendItem(ChooseFriendDialog *owner,
                                   const QString &userId,
                                   const QIcon &avatar,
                                   const QString &name,
                                   bool checked,
                                   bool readOnly)
    : userId(userId)
    , readOnly(readOnly)
{
    // 控件基本属性
    setFixedHeight(50);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(15); // 增加间距
    layout->setContentsMargins(15, 0, 15, 0); // 调整边距
    setLayout(layout);

    // 复选框
    checkBox = new QCheckBox();
    checkBox->setChecked(checked);
    checkBox->setFixedSize(25, 25);
    checkBox->setCursor(Qt::PointingHandCursor);

    // 如果 readOnly = true，禁用 checkBox
    checkBox->setEnabled(!readOnly);

    static const QString style =
        "QCheckBox {"
        "    background-color: transparent;"
        "    spacing: 8px;"
        "}"
        "QCheckBox::indicator {"
        "    width: 20px;"
        "    height: 20px;"
        "    image: url(:/resource/image/unchecked.png);"
        "}"
        "QCheckBox::indicator:checked {"
        "    image: url(:/resource/image/checked.png);"
        "}";
    checkBox->setStyleSheet(style);

    // 头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(36, 36);
    avatarBtn->setIconSize(QSize(36, 36));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    border-radius: 5px;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(208, 221, 247, 0.3);"
        "}");

    // 名字
    nameLabel = new QLabel();
    nameLabel->setText(name);
    nameLabel->setStyleSheet(
        "QLabel {"
        "    background-color: transparent;"
        "    color: rgb(80, 80, 80);"
        "    font-size: 14px;"
        "}");

    // 添加内容到布局管理器中
    layout->addWidget(checkBox);
    layout->addWidget(avatarBtn);
    layout->addWidget(nameLabel);
    layout->addStretch();


    // 连接信号槽
    connect(checkBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (readOnly)
        {
            Toast::showMessage("该成员必须选中!");
            return;
        }

        if (checked)
            owner->addSelectedFriend(userId, avatar, name, false);
        else
            owner->deleteSelectedFriend(userId);
    });
}

void ChooseFriendItem::paintEvent(QPaintEvent *event)
{
    (void) event;

    QPainter painter(this);
    QColor color = isHover ? QColor(240, 243, 250) : QColor(255, 255, 255);
    painter.fillRect(rect(), color);
}

// 保持enterEvent和leaveEvent不变
void ChooseFriendItem::enterEvent(QEnterEvent *event)
{
    (void) event;
    isHover = true;
    update();
}

void ChooseFriendItem::leaveEvent(QEvent *event)
{
    (void) event;
    isHover =  false;
    update();
}

const QString &ChooseFriendItem::getUserId() const
{
    return userId;
}

QCheckBox *ChooseFriendItem::getCheckBox()
{
    return checkBox;
}

bool ChooseFriendItem::isReadOnly()
{
    return readOnly;
}

ChooseFriendDialog::ChooseFriendDialog(QWidget *parent,
                                       const QList<QString> & userIds,
                                       bool isSingelSession)
    : QDialog(parent)
    , userIds(userIds)
    , isSingelSession(isSingelSession)
{
    // 窗口基本属性
    setWindowTitle("选择好友");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setFixedSize(750, 550);
    setStyleSheet(
        "QDialog {"
        "    background-color: rgb(245, 245, 245);"
        "}");
    setAttribute(Qt::WA_DeleteOnClose);

    // 布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    initLeft(layout); // 左侧窗口初始化
    initRight(layout); // 右侧窗口初始化
    initData(); // 加载数据
}

void ChooseFriendDialog::initLeft(QHBoxLayout *layout)
{
    // 创建一个外层容器，设置灰色背景
    QWidget* leftOuterContainer = new QWidget();
    leftOuterContainer->setStyleSheet(
        "background-color: rgb(245, 245, 245);"
        "border-radius: 5px;"
        );
    QVBoxLayout* outerLayout = new QVBoxLayout(leftOuterContainer);
    outerLayout->setContentsMargins(20, 20, 20, 20);
    outerLayout->setSpacing(0);
    layout->addWidget(leftOuterContainer, 1);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    background-color: transparent;"
        "    border: 2px solid rgb(230, 230, 230);"
        "}"
        "QScrollBar:vertical {"
        "    width: 6px;"
        "    background: transparent;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgb(208, 221, 247);"
        "    min-height: 20px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "    background: none;"
        "}"
        );
    outerLayout->addWidget(scrollArea);

    // 白色内容区容器
    QWidget* whiteContainer = new QWidget();
    whiteContainer->setStyleSheet(
        "background-color: rgb(255, 255, 255);"
        "border-radius: 5px;"
        );
    scrollArea->setWidget(whiteContainer);

    // 内容容器
    totalContainer = new QWidget();
    totalContainer->setObjectName("totalContainer");
    totalContainer->setStyleSheet(
        "#totalContainer {"
        "    background-color: rgb(255, 255, 255);"
        "}"
        );

    // 设置布局
    QVBoxLayout* containerLayout = new QVBoxLayout(whiteContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(totalContainer);

    // 左侧子窗口内部的垂直布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    totalContainer->setLayout(vlayout);

#if TEST_UI
    QIcon defaultAvatar(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 30; ++i)
        addFriend(QString::number(1000 + i), defaultAvatar, "张三" + QString::number(i), false);
#endif
}

void ChooseFriendDialog::initRight(QHBoxLayout *layout)
{
    // 右侧布局管理器
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(15, 15, 15, 15);
    gridLayout->setSpacing(15);
    layout->addLayout(gridLayout, 1);

    // 提示 label
    QLabel* tipLabel = new QLabel("选择联系人");
    tipLabel->setFixedHeight(30);
    tipLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tipLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tipLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: rgb(80, 80, 80);"
        "    padding-left: 5px;"
        "}");

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "    width: 6px;"
        "    background: rgb(240, 243, 250);"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgb(208, 221, 247);"
        "    min-height: 20px;"
        "    border-radius: 3px;"
        "}");
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border-radius: 5px;"
        "    border: 2px solid rgb(230, 230, 230);"
        "    background-color: rgb(255, 255, 255);"
        "}");
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 滚动区域的 QWidget
    selectedContainer = new QWidget();
    selectedContainer->setObjectName("selectedContainer");
    selectedContainer->setStyleSheet("#selectedContainer { background-color: rgb(255, 255, 255); }");
    scrollArea->setWidget(selectedContainer);

    // selectedContainer 的垂直布局
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    selectedContainer->setLayout(vlayout);

    // 底部按钮
    QPushButton* okBtn = new QPushButton("完成");
    static const QString style =
        "QPushButton {"
        "    color: rgb(255, 255, 255);"
        "    background-color: rgb(100, 130, 180);"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-size: 14px;"
        "    padding: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(80, 110, 160);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(60, 90, 140);"
        "}";
    okBtn->setFixedHeight(40);
    okBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    okBtn->setStyleSheet(style);
    okBtn->setCursor(Qt::PointingHandCursor);

    QPushButton* cancelBtn = new QPushButton("取消");
    static const QString cancelStyle =
        "QPushButton {"
        "    color: rgb(100, 130, 180);"
        "    background-color: rgb(240, 243, 250);"
        "    border: 1px solid rgb(208, 221, 247);"
        "    border-radius: 5px;"
        "    font-size: 14px;"
        "    padding: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(230, 238, 255);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(208, 221, 247);"
        "}";

    cancelBtn->setFixedHeight(40);
    cancelBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cancelBtn->setStyleSheet(cancelStyle);
    cancelBtn->setCursor(Qt::PointingHandCursor);

    // 添加控件到布局中
    gridLayout->addWidget(tipLabel, 0, 0, 1, 9);
    gridLayout->addWidget(scrollArea, 1, 0, 1, 9);
    gridLayout->addWidget(okBtn, 2, 1, 1, 3);
    gridLayout->addWidget(cancelBtn, 2, 5, 1, 3);

    // 信号槽, 处理 ok 和 cancel 的点击
    connect(okBtn, &QPushButton::clicked, this, &ChooseFriendDialog::clickOkBtn);
    connect(cancelBtn, &QPushButton::clicked, this, [=]() {
        close();
    });
}

void ChooseFriendDialog::clickOkBtn()
{
    QList<QString> userIdList = generateMemberList();


    // 发送请求
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (isSingelSession)
    {
        if (userIdList.size() < 3)
        {
            Toast::showMessage("尚未添加任何新的群成员");
            return;
        }

        dataCenter->createGroupChatSessionAsync(userIdList);
    }
    else
    {
        if (userIdList.size() <= 0)
        {
            Toast::showMessage("尚未添加任何新的群成员");
            return;
        }

        dataCenter->groupAddMemberAsync(userIdList);
    }

    close();
}

QList<QString> ChooseFriendDialog::generateMemberList()
{
    QList<QString> result;

    if (isSingelSession)
    {

        IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
        if (dataCenter->getMyself() == nullptr)
        {
            LOG() << "个人信息尚未加载!";
            return result;
        }
        result.push_back(dataCenter->getMyself()->userId); // 把自己添加到结果中
        result.push_back(userIds[0]); // 把好友添加到结果中
    }

    // 遍历选中列表
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(selectedContainer->layout());
    for (int i = 0; i < layout->count(); ++i)
    {
        auto* item = layout->itemAt(i);
        if (item == nullptr || item->widget() == nullptr)
            continue;

        auto* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if (chooseFriendItem->isReadOnly()) // 成员已经存在
            continue;

        result.push_back(chooseFriendItem->getUserId());
    }

    return result;
}

void ChooseFriendDialog::initData()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<IM::Model::UserInfo>* friendList = dataCenter->getFriendList();
    if (friendList == nullptr)
    {
        LOG() << "加载数据时发现好友列表为空!";
        return;
    }

    for (auto it = friendList->begin(); it != friendList->end(); ++it)
    {
        if (userIds.contains(it->userId))
        {
            addSelectedFriend(it->userId, it->avatar, it->nickname, true);
            addFriend(it->userId, it->avatar, it->nickname, true, true);
        }
        else
        {
            addFriend(it->userId, it->avatar, it->nickname, false, false);
        }
    }
}

void ChooseFriendDialog::addFriend(const QString &userId,
                                   const QIcon &avatar,
                                   const QString &name,
                                   bool checked,
                                   bool readOnly)
{
    ChooseFriendItem* item = new ChooseFriendItem(this, userId, avatar, name, checked, readOnly);
    totalContainer->layout()->addWidget(item);
}

void ChooseFriendDialog::addSelectedFriend(const QString &userId,
                                           const QIcon &avatar,
                                           const QString &name,
                                           bool readOnly)
{
    ChooseFriendItem* item = new ChooseFriendItem(this, userId, avatar, name, true, readOnly);
    selectedContainer->layout()->addWidget(item);
}

void ChooseFriendDialog::deleteSelectedFriend(const QString &userId)
{
    QVBoxLayout* vlayout = dynamic_cast<QVBoxLayout*>(selectedContainer->layout());

    for (int i = vlayout->count() - 1; i >= 0; --i)
    {
        auto* item = vlayout->itemAt(i);
        if (item == nullptr || item->widget() == nullptr)
            continue;

        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if (chooseFriendItem->getUserId() != userId)
            continue;

        vlayout->removeWidget(chooseFriendItem);
        chooseFriendItem->deleteLater(); // QT 正在使用该控件，不能直接delete
        break;
    }

    // 遍历左侧列表,取消对应 item 的 checkBox 勾选状态
    QVBoxLayout* vlayoutLeft = dynamic_cast<QVBoxLayout*>(totalContainer->layout());
    for (int i = 0; i < vlayoutLeft->count(); ++i)
    {
        auto* item = vlayoutLeft->itemAt(i);
        if (item == nullptr || item->widget() == nullptr)
            continue;

        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if (chooseFriendItem->getUserId() != userId)
            continue;

        chooseFriendItem->getCheckBox()->setChecked(false); // 取消 checkBox 选中状态
        break;
    }
}
