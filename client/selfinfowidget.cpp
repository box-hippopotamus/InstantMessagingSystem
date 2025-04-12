#include "selfinfowidget.h"

SelfInfoWidget::SelfInfoWidget(QWidget *parent)
    : QDialog(parent)
{
    // 设置整个窗口的属性
    setFixedSize(500, 200);
    setWindowTitle("个人信息");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setAttribute(Qt::WA_DeleteOnClose);
    move(QCursor::pos());
    setStyleSheet("QDialog { background-color: rgb(245, 245, 245); }");

    // 布局管理器
    layout = new QGridLayout();
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(3);
    layout->setContentsMargins(20, 20, 20, 0);
    layout->setAlignment(Qt::AlignTop);
    setLayout(layout);

    // 创建头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(75, 75);
    avatarBtn->setIconSize(QSize(75, 75));
    avatarBtn->setStyleSheet(
        "QPushButton {"
        " border: 2px solid rgb(208, 221, 247);"
        " border-radius: 5px;"
        " background-color: white;"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(240, 243, 250);"
        "}");
    layout->addWidget(avatarBtn, 0, 0, 3, 1);

    // 通用样式定义
    static const QString labelStyle = "QLabel {"
                                      " font-size: 14px;"
                                      " font-weight: 600;"
                                      " color: rgb(80, 80, 80);"
                                      "}";

    static const QString btnStyle = "QPushButton {"
                                    " border: none;"
                                    " background-color: transparent;"
                                    " color: rgb(100, 130, 180);"
                                    " border-radius: 4px;"
                                    " padding: 2px 5px;"
                                    "}"
                                    "QPushButton:pressed {"
                                    " background-color: rgb(208, 221, 247);"
                                    "}";

    static const QString editStyle = "QLineEdit {"
                                     " border: 1px solid rgb(208, 221, 247);"
                                     " border-radius: 5px;"
                                     " padding-left: 5px;"
                                     " background-color: white;"
                                     "}"
                                     "QLineEdit:focus {"
                                     " border: 1px solid rgb(150, 180, 220);"
                                     "}";

    static const int height = 30;

    // 用户ID
    idTag = new QLabel("ID");
    idTag->setFixedSize(50, height);
    idTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    idTag->setStyleSheet(labelStyle);

    idLabel = new QLabel();
    idLabel->setFixedHeight(height);
    idLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    idLabel->setStyleSheet("QLabel { color: rgb(120, 120, 120); }");

    // 用户名
    nameTag = new QLabel("昵称");
    nameTag->setFixedSize(50, height);
    nameTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nameTag->setStyleSheet(labelStyle);

    nameLabel = new QLabel();
    nameLabel->setFixedHeight(height);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setStyleSheet("QLabel { color: rgb(120, 120, 120); }");

    nameModifyBtn = new QPushButton();
    nameModifyBtn->setFixedSize(70, 25);
    nameModifyBtn->setIconSize(QSize(20, 20));
    nameModifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    nameModifyBtn->setStyleSheet(btnStyle);

    nameEdit = new QLineEdit();
    nameEdit->setFixedHeight(height);
    nameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameEdit->setStyleSheet(editStyle);
    nameEdit->hide();

    nameSubmitBtn = new QPushButton("保存");
    nameSubmitBtn->setFixedSize(70, 25);
    nameSubmitBtn->setStyleSheet(
        "QPushButton {"
        " border: 1px solid rgb(180, 200, 240);"
        " border-radius: 4px;"
        " background-color: rgb(208, 221, 247);"
        " color: rgb(80, 80, 80);"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(180, 200, 240);"
        "}");
    nameSubmitBtn->hide();

    // 个性签名
    descTag = new QLabel("签名");
    descTag->setFixedSize(50, height);
    descTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    descTag->setStyleSheet(labelStyle);

    descLabel = new QLabel();
    descLabel->setFixedHeight(height);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descLabel->setStyleSheet("QLabel { color: rgb(120, 120, 120); }");

    descModifyBtn = new QPushButton();
    descModifyBtn->setFixedSize(70, 25);
    descModifyBtn->setIconSize(QSize(20, 20));
    descModifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    descModifyBtn->setStyleSheet(btnStyle);

    descEdit = new QLineEdit();
    descEdit->setFixedHeight(height);
    descEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descEdit->setStyleSheet(editStyle);
    descEdit->hide();

    descSubmitBtn = new QPushButton("保存");
    descSubmitBtn->setFixedSize(70, 25);
    descSubmitBtn->setStyleSheet(
        "QPushButton {"
        " border: 1px solid rgb(180, 200, 240);"
        " border-radius: 4px;"
        " background-color: rgb(208, 221, 247);"
        " color: rgb(80, 80, 80);"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(180, 200, 240);"
        "}");
    descSubmitBtn->hide();

    // 电话
    phoneTag = new QLabel("电话");
    phoneTag->setFixedSize(50, height);
    phoneTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    phoneTag->setStyleSheet(labelStyle);

    phoneLabel = new QLabel();
    phoneLabel->setFixedHeight(height);
    phoneLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    phoneLabel->setStyleSheet("QLabel { color: rgb(120, 120, 120); }");

    phoneModifyBtn = new QPushButton();
    phoneModifyBtn->setFixedSize(70, 25);
    phoneModifyBtn->setIconSize(QSize(20, 20));
    phoneModifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    phoneModifyBtn->setStyleSheet(btnStyle);

    phoneEdit = new QLineEdit();
    phoneEdit->setFixedHeight(height);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    phoneEdit->setStyleSheet(editStyle);
    phoneEdit->hide();

    phoneSubmitBtn = new QPushButton("保存");
    phoneSubmitBtn->setFixedSize(70, 25);
    phoneSubmitBtn->setStyleSheet(
        "QPushButton {"
        " border: 1px solid rgb(180, 200, 240);"
        " border-radius: 4px;"
        " background-color: rgb(208, 221, 247);"
        " color: rgb(80, 80, 80);"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(180, 200, 240);"
        "}");
    phoneSubmitBtn->hide();

    // 验证码
    verifyCodeTag = new QLabel("验证码");
    verifyCodeTag->setFixedSize(50, height);
    verifyCodeTag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    verifyCodeTag->setStyleSheet(labelStyle);
    verifyCodeTag->hide();

    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(height);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    verifyCodeEdit->setStyleSheet(editStyle);
    verifyCodeEdit->hide();

    getVerifyCodeBtn = new QPushButton("获取验证码");
    getVerifyCodeBtn->setFixedSize(70, height);
    getVerifyCodeBtn->setStyleSheet(
        "QPushButton {"
        " border: 1px solid rgb(208, 221, 247);"
        " border-radius: 4px;"
        " background-color: rgb(240, 243, 250);"
        " color: rgb(100, 130, 180);"
        "}"
        "QPushButton:pressed {"
        " background-color: rgb(208, 221, 247);"
        "}");
    getVerifyCodeBtn->hide();

    // 添加到布局管理器
    layout->addWidget(idTag, 0, 1);
    layout->addWidget(idLabel, 0, 2);

    layout->addWidget(nameTag, 1, 1);
    layout->addWidget(nameLabel, 1, 2);
    layout->addWidget(nameModifyBtn, 1, 3);
    layout->addWidget(nameEdit, 1, 2);
    layout->addWidget(nameSubmitBtn, 1, 3);

    layout->addWidget(descTag, 2, 1);
    layout->addWidget(descLabel, 2, 2);
    layout->addWidget(descModifyBtn, 2, 3);
    layout->addWidget(descEdit, 2, 2);
    layout->addWidget(descSubmitBtn, 2, 3);

    layout->addWidget(phoneTag, 3, 1);
    layout->addWidget(phoneLabel, 3, 2);
    layout->addWidget(phoneModifyBtn, 3, 3);
    layout->addWidget(phoneEdit, 3, 2);
    layout->addWidget(phoneSubmitBtn, 3, 3);
    layout->addWidget(verifyCodeTag, 4, 1);
    layout->addWidget(verifyCodeEdit, 4, 2);
    layout->addWidget(getVerifyCodeBtn, 4, 3);


#if TEST_UI
    idLabel->setText("1234");
    nameLabel->setText("张三");
    descLabel->setText("从今天开始认真敲代码");
    phoneLabel->setText("18612345678");
    avatarBtn->setIcon(QIcon(":/resource/image/defaultAvatar.png"));
#endif

    // 连接信号槽
    initSingalSlot();

    // 加载个人数据
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    IM::Model::UserInfo* myself = dataCenter->getMyself();
    if (myself != nullptr)
    {
        avatarBtn->setIcon(myself->avatar);
        idLabel->setText(myself->userId);
        nameLabel->setText(myself->nickname);
        descLabel->setText(myself->description);
        phoneLabel->setText(myself->phone);
    }
}

void SelfInfoWidget::initSingalSlot()
{
    connect(nameModifyBtn, &QPushButton::clicked, this, [this]() {
        // 把当前的 nameLabel 和 nameModifyBtn 隐藏起来
        nameLabel->hide();
        nameModifyBtn->hide();
        layout->removeWidget(nameLabel);
        layout->removeWidget(nameModifyBtn);
        // 把 nameEdit 和 nameSubmitBtn 显示出来
        nameEdit->show();
        nameSubmitBtn->show();
        layout->addWidget(nameEdit, 1, 2);
        layout->addWidget(nameSubmitBtn, 1, 3);
        // 把输入框的内容进行设置.
        nameEdit->setText(nameLabel->text());
    });

    connect(descModifyBtn, &QPushButton::clicked, this, [this]() {
        descLabel->hide();
        descModifyBtn->hide();
        layout->removeWidget(descLabel);
        layout->removeWidget(descModifyBtn);

        descEdit->show();
        descSubmitBtn->show();
        layout->addWidget(descEdit, 2, 2);
        layout->addWidget(descSubmitBtn, 2, 3);

        descEdit->setText(descLabel->text());
    });

    connect(phoneModifyBtn, &QPushButton::clicked, this, [this]() {
        phoneLabel->hide();
        phoneModifyBtn->hide();
        layout->removeWidget(phoneLabel);
        layout->removeWidget(phoneModifyBtn);

        phoneEdit->show();
        phoneSubmitBtn->show();
        layout->addWidget(phoneEdit, 3, 2);
        layout->addWidget(phoneSubmitBtn, 3, 3);

        verifyCodeTag->show();
        verifyCodeEdit->show();
        getVerifyCodeBtn->show();
        layout->addWidget(verifyCodeTag, 4, 1);
        layout->addWidget(verifyCodeEdit, 4, 2);
        layout->addWidget(getVerifyCodeBtn, 4, 3);

        phoneEdit->setText(phoneLabel->text());
    });

    connect(nameSubmitBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickNameSubmitBtn);
    connect(descSubmitBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickDescSubmitBtn);
    connect(getVerifyCodeBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickGetVerifyCodeBtn);
    connect(phoneSubmitBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickPhoneSubmitBtn);
    connect(avatarBtn, &QPushButton::clicked, this, &SelfInfoWidget::clickAvatarBtn);
}


void SelfInfoWidget::clickNameSubmitBtn()
{
    // 获取修改后的昵称
    const QString& nickname = nameEdit->text();
    if (nickname.isEmpty())
        return;

    // 发送请求
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::changeNicknameDone, this, &SelfInfoWidget::clickNameSubmitBtnDone, Qt::UniqueConnection);
    dataCenter->changeNicknameAsync(nickname);
}

void SelfInfoWidget::clickNameSubmitBtnDone()
{
    // 对界面控件进行切换. 把刚才输入框切换回 label, 把提交按钮切换回编辑按钮
    layout->removeWidget(nameEdit);
    nameEdit->hide();
    layout->addWidget(nameLabel, 1, 2);
    nameLabel->show();
    nameLabel->setText(nameEdit->text());

    layout->removeWidget(nameSubmitBtn);
    nameSubmitBtn->hide();
    layout->addWidget(nameModifyBtn, 1, 3);
    nameModifyBtn->show();
}

void SelfInfoWidget::clickDescSubmitBtn()
{
    // 获取签名内容
    const QString& desc = descEdit->text();
    if (desc.isEmpty())
        return;

    // 发送网络请求
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::changeDescriptionDone, this, &SelfInfoWidget::chickDescSubmitBtnDone, Qt::UniqueConnection);
    dataCenter->changeDescriptionAsync(desc);
}

void SelfInfoWidget::chickDescSubmitBtnDone()
{
    // 切换界面
    layout->removeWidget(descEdit);
    descEdit->hide();
    layout->addWidget(descLabel, 2, 2);
    descLabel->show();
    descLabel->setText(descEdit->text());

    layout->removeWidget(descSubmitBtn);
    descSubmitBtn->hide();
    layout->addWidget(descModifyBtn, 2, 3);
    descModifyBtn->show();
}

void SelfInfoWidget::clickGetVerifyCodeBtn()
{
    // 获取到输入框中的手机号码
    const QString& phone = phoneEdit->text();
    if (phone.isEmpty())
        return;

    // 发起请求.
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::getVerifyCodeDone, this, [=]() {
        Toast::showMessage("短信验证码已经发送");
    });
    dataCenter->getVerifyCodeAsync(phone);

    // 刚才发送请求的手机号码, 保存起来
    phoneToChange = phone;

    // 禁用发送验证码按钮, 倒计时
    getVerifyCodeBtn->setEnabled(false);

    leftTime = 30;
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        if (leftTime <= 1)
        {
            // 倒计时结束
            getVerifyCodeBtn->setEnabled(true);
            getVerifyCodeBtn->setText("获取验证码");
            timer->stop();
            timer->deleteLater();
            return;
        }
        --leftTime;
        getVerifyCodeBtn->setText(QString::number(leftTime) + "s");
    });
    timer->start(1000);
}

void SelfInfoWidget::clickPhoneSubmitBtn()
{
    // 判定, 当前验证码是否已经收到.
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QString verifyCodeId = dataCenter->getVerifyCodeId();
    if (verifyCodeId.isEmpty())
    {
        Toast::showMessage("服务器尚未返回响应, 稍后重试!");
        return;
    }

    // 清空 DataCenter 中存储的值
    dataCenter->resetVerifyCodeId("");

    // 获取到用户输入的验证码
    QString verifyCode = verifyCodeEdit->text();
    if (verifyCode.isEmpty())
    {
        Toast::showMessage("验证码不能为空!");
        return;
    }

    verifyCodeEdit->setText("");  // 获取到验证码之后, 就可以清空了.

    // 发送请求, 把当前验证码信息, 发送给服务器
    connect(dataCenter, &IM::Model::DataCenter::changePhoneDone, this, &SelfInfoWidget::clickPhoneSubmitBtnDone, Qt::UniqueConnection);
    dataCenter->changePhoneAsync(this->phoneToChange, verifyCodeId, verifyCode);

    // 停止验证码按钮倒计时
    leftTime = 1;
}

void SelfInfoWidget::clickPhoneSubmitBtnDone()
{
    layout->removeWidget(verifyCodeTag);
    layout->removeWidget(verifyCodeEdit);
    layout->removeWidget(getVerifyCodeBtn);
    layout->removeWidget(phoneEdit);
    layout->removeWidget(phoneSubmitBtn);
    verifyCodeTag->hide();
    verifyCodeEdit->hide();
    getVerifyCodeBtn->hide();
    phoneEdit->hide();
    phoneSubmitBtn->hide();

    layout->addWidget(phoneLabel, 3, 2);
    phoneLabel->show();
    phoneLabel->setText(this->phoneToChange);
    layout->addWidget(phoneModifyBtn, 3, 3);
    phoneModifyBtn->show();
}

void SelfInfoWidget::clickAvatarBtn()
{
    // 选择文件
    QString filter = "Image Files (*.png *.jpg *.jpeg)";
    QString imagePath = QFileDialog::getOpenFileName(this, "选择头像", QDir::homePath(), filter);
    if (imagePath.isEmpty())
    {
        LOG() << "用户未选择任何头像";
        return;
    }

    // 读取图片
    QByteArray imageBytes = IM::loadFileToByteArray(imagePath);

    // 发送请求, 修改头像
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::changeAvatarDone, this, &SelfInfoWidget::clickAvatarBtnDone, Qt::UniqueConnection);
    dataCenter->changeAvatarAsync(imageBytes);
}

void SelfInfoWidget::clickAvatarBtnDone()
{
    // 更新设置的头像
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    avatarBtn->setIcon(dataCenter->getMyself()->avatar);
}
