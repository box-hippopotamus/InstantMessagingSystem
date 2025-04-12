#include "groupsessiondetailwidget.h"

GroupSessionDetailWidget::GroupSessionDetailWidget(QWidget *parent)
    : QDialog(parent)
{
    // 窗口基本属性
    setFixedSize(375, 550);
    setWindowTitle("群组详情");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setStyleSheet("QDialog { background-color: rgb(245, 245, 245); }");
    setAttribute(Qt::WA_DeleteOnClose);

    // 主布局
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(5);
    vlayout->setContentsMargins(20, 20, 20, 20);
    setLayout(vlayout);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    background-color: rgb(255, 255, 255);"
        "    border: 1px solid rgb(230, 230, 230);"
        "    border-radius: 5px;"
        "}"
        "QScrollBar:vertical {"
        "    width: 5px;"
        "    background: rgb(240, 243, 250);"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgb(208, 221, 247);"
        "    min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}");
    scrollArea->setFixedSize(335, 350);

    // 容器
    QWidget* container = new QWidget();
    container->setStyleSheet("background-color: rgb(255, 255, 255);");
    scrollArea->setWidget(container);

    glayout = new QGridLayout();
    glayout->setContentsMargins(15, 15, 15, 15);
    glayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    container->setLayout(glayout);

    vlayout->addWidget(scrollArea);

    // 添加按钮
    addBtn = new AvatarItem(QIcon(":/resource/image/cross.png"), "添加");
    glayout->addWidget(addBtn, 0, 0);

    // 群聊名称标签
    QLabel* groupNameTag = new QLabel("群聊名称");
    groupNameTag->setStyleSheet(
        "QLabel {"
        "    font-weight: 600;"
        "    font-size: 16px;"
        "    color: rgb(80, 80, 80);"
        "}");
    groupNameTag->setFixedHeight(20);
    vlayout->addWidget(groupNameTag);

    // 群名和修改按钮布局
    QHBoxLayout* hlayout = new QHBoxLayout();

    groupNameLabel = new QLabel();
    groupNameLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    color: rgb(50, 50, 50);"
        "}");
    hlayout->addWidget(groupNameLabel);

    groupNameEdit = new QLineEdit();
    groupNameEdit->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    color: rgb(50, 50, 50);"
        "}");
    hlayout->addWidget(groupNameEdit);
    groupNameEdit->hide();

    // 修改按钮
    modifyBtn = new QPushButton();
    modifyBtn->setFixedSize(30, 30);
    modifyBtn->setIconSize(QSize(20, 20));
    modifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    modifyBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    background-color: transparent;"
        "}");
    modifyBtn->setCursor(Qt::PointingHandCursor);
    hlayout->addWidget(modifyBtn);


    vlayout->addLayout(hlayout);

    // 退出群聊按钮
    exitGroupBtn = new QPushButton("退出群聊");
    exitGroupBtn->setFixedHeight(30);
    exitGroupBtn->setFixedWidth(200);
    exitGroupBtn->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid rgb(240, 150, 150);"
        "    border-radius: 5px;"
        "    background-color: rgb(250, 230, 230);"
        "    color: rgb(180, 80, 80);"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(250, 210, 210);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(240, 150, 150);"
        "}");
    exitGroupBtn->setCursor(Qt::PointingHandCursor);
    vlayout->addWidget(exitGroupBtn, 0, Qt::AlignHCenter);

#if TEST_UI
    groupNameLabel->setText("人类吃喝行为研究小组");
    QIcon avatar(":/resource/image/defaultAvatar.png");
    for (int i = 0; i < 20; ++i)
    {
        AvatarItem* item = new AvatarItem(avatar, "张三" + QString::number(i));
        addMember(item);
    }
#endif

    initSignalSlot();
    // 加载数据
    initData();
}

void GroupSessionDetailWidget::initSignalSlot()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();

    connect(modifyBtn, &QPushButton::clicked, this, [=](){
        if (groupNameEdit->isHidden())
        {
            groupNameEdit->show();
            groupNameLabel->hide();
        }
        else
        {
            groupNameEdit->hide();
            groupNameLabel->show();
        }
    });

    connect(exitGroupBtn, &QPushButton::clicked, this, [=](){
        dataCenter->leaveGroupAsync(dataCenter->getCurrentChatSessionId());
    });

    connect(dataCenter, &IM::Model::DataCenter::leaveGroupDone, this, [=](){
        close();
    });

    // 修改群名
    connect(groupNameEdit, &QLineEdit::editingFinished, this, [=](){
        modifyName = groupNameEdit->text();
        dataCenter->groupModifyNameAsync(modifyName);
    });

    connect(dataCenter, &IM::Model::DataCenter::groupModifyNameDone, this, [=](){
        groupNameLabel->setText(modifyName);
        groupNameEdit->setPlaceholderText(modifyName);
        Toast::showMessage("修改群名成功!");
    });

    // 添加成员
    connect(addBtn->getAvatar(), &QPushButton::clicked, this, [=]() {
        QList<UserInfo>* memberList = dataCenter->getMemberList(dataCenter->getCurrentChatSessionId());
        QList<QString> userIds;
        for (auto& member : *memberList)
            userIds.push_back(member.userId);

        ChooseFriendDialog* chooseFriendDialog = new ChooseFriendDialog(this, userIds, false);
        chooseFriendDialog->exec();
    });

    // 添加成员完毕
    connect(dataCenter, &IM::Model::DataCenter::groupAddMemberDone, this, [=]() {
        initMembers(dataCenter->getCurrentChatSessionId());
    });
}

void GroupSessionDetailWidget::initData()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::getMemberListDone, this, &GroupSessionDetailWidget::initMembers);
    dataCenter->getMemberListAsync(dataCenter->getCurrentChatSessionId());
}

void GroupSessionDetailWidget::initMembers(const QString& chatSessionId)
{
    total = 0;

    // 根据刚才拿到的成员列表, 把成员列表渲染到界面上
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<UserInfo>* memberList = dataCenter->getMemberList(chatSessionId);
    if (memberList == nullptr)
    {
        LOG() << "获取的成员列表为空! chatSessionId=" << chatSessionId;
        return;
    }

    // 遍历成员列表
    for (const auto& u : *memberList)
    {
        AvatarItem* avatarItem = new AvatarItem(u.avatar, u.nickname);
        addMember(avatarItem);
    }

    IM::Model::ChatSessionInfo* info = dataCenter->findChatSessionById(chatSessionId);
    groupNameEdit->setPlaceholderText(info->chatSessionName);
    groupNameLabel->setText(info->chatSessionName);
}

void GroupSessionDetailWidget::addMember(AvatarItem *avatarItem)
{
    static const int MAX_COL = 4;

    ++total; // 先自增，刚好空出 0 号位给添加按钮
    glayout->addWidget(avatarItem, total / MAX_COL, total % MAX_COL);
}
