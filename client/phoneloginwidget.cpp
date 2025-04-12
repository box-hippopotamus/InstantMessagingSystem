#include "phoneloginwidget.h"

PhoneLoginWidget::PhoneLoginWidget(QWidget *parent)
    : QWidget{parent}
{
    // 基本属性
    setFixedSize(300, 380);
    setWindowTitle("登录");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setStyleSheet(
        "QWidget {"
        "   background-color: rgb(245, 245, 245);"
        "   font-family: Microsoft YaHei;"
        "}");
    setAttribute(Qt::WA_DeleteOnClose);

    // 布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(12);
    layout->setContentsMargins(40, 30, 40, 30);
    setLayout(layout);

    // 标题
    titleLabel = new QLabel();
    titleLabel->setText("登录");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(60);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 25px;"
        "   font-weight: 500;"
        "   color: rgb(50, 55, 65);"
        "}");

    // 输入框样式
    static const QString editStyle =
        "QLineEdit {"
        "   border: 1px solid rgb(208, 221, 247);"
        "   border-radius: 8px;"
        "   font-size: 14px;"
        "   background-color: rgb(240, 243, 250);"
        "   padding: 8px 12px;"
        "   color: rgb(50, 55, 65);"
        "}"
        "QLineEdit:focus {"
        "   border-color: rgb(180, 190, 210);"
        "}";

    // 手机号输入框
    phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText("输入手机号");
    phoneEdit->setFixedHeight(35);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    phoneEdit->setStyleSheet(editStyle);

    // 验证码输入框
    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(35);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    verifyCodeEdit->setPlaceholderText("输入验证码");
    verifyCodeEdit->setStyleSheet(editStyle);

    // 蓝色透明按钮
    static const QString btnTransparentBlueStyle =
        "QPushButton {"
        "   border: 1px solid rgba(208, 221, 247, 80);"
        "   border-radius: 8px;"
        "   background-color: rgba(240, 243, 250, 60);"
        "   color: rgb(80, 90, 110);"
        "   font-size: 13px;"
        "   padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(208, 221, 247, 30);"
        "   border-color: rgba(208, 221, 247, 120);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(208, 221, 247, 50);"
        "   border-color: rgba(180, 190, 210, 150);"
        "}"
        "QPushButton:disabled {"
        "   color: rgb(180, 190, 210);"
        "   border-color: rgba(180, 190, 210, 50);"
        "   background-color: rgba(240, 243, 250, 20);"
        "}";

    sendVerifyCodeBtn = new QPushButton();
    sendVerifyCodeBtn->setFixedSize(100, 40);
    sendVerifyCodeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sendVerifyCodeBtn->setText("发送验证码");
    sendVerifyCodeBtn->setStyleSheet(btnTransparentBlueStyle);

    // 提交按钮
    static const QString btnBlueStyle =
        "QPushButton {"
        "   border: none;"
        "   border-radius: 8px;"
        "   background-color: rgb(208, 221, 247);"
        "   color: rgb(50, 55, 65);"
        "   font-size: 14px;"
        "   padding: 8px;"
        "   font-weight: 500;"
        "   border: 1px solid rgba(180, 190, 210, 150);"
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(200, 215, 240);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgb(190, 205, 230);"
        "   border-color: rgba(160, 175, 200, 150);"
        "}"
        "QPushButton:disabled {"
        "   background-color: rgb(230, 235, 245);"
        "   color: rgb(180, 190, 210);"
        "}";

    submitBtn = new QPushButton();
    submitBtn->setFixedHeight(35);
    submitBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    submitBtn->setText("登录");
    submitBtn->setStyleSheet(btnBlueStyle);

    // 切换到用户名
    QPushButton* userModeBtn = new QPushButton();
    userModeBtn->setFixedSize(100, 36);
    userModeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    userModeBtn->setText("切换到用户名");
    userModeBtn->setStyleSheet(btnTransparentBlueStyle);

    // 切换登录注册模式
    switchModeBtn = new QPushButton();
    switchModeBtn->setFixedSize(100, 36);
    switchModeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    switchModeBtn->setText("注册");
    switchModeBtn->setStyleSheet(btnTransparentBlueStyle);

    // 添加到布局管理器
    layout->addWidget(titleLabel, 0, 0, 1, 5);
    layout->addWidget(phoneEdit, 1, 0, 1, 5);
    layout->addWidget(verifyCodeEdit, 2, 0, 1, 4);
    layout->addWidget(sendVerifyCodeBtn, 2, 4, 1, 1);
    layout->addWidget(submitBtn, 3, 0, 1, 5);
    layout->addWidget(userModeBtn, 4, 0, 1, 1);
    layout->addWidget(switchModeBtn, 4, 4, 1, 1);

    // 信号槽
    connect(switchModeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::switchMode);

    connect(userModeBtn, &QPushButton::clicked, this, [this]() {
        LoginWidget* loginWidget = new LoginWidget(nullptr);
        loginWidget->show();
        close();
    });

    connect(sendVerifyCodeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::sendVerifyCode);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PhoneLoginWidget::countDown);

    connect(submitBtn, &QPushButton::clicked, this, &PhoneLoginWidget::clickSubmitBtn);
}

void PhoneLoginWidget::sendVerifyCode()
{
    const QString& phone = this->phoneEdit->text();
    if (phone.isEmpty())
        return;

    currentPhone = phone;

    // 发送网络请求, 获取验证码
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::getVerifyCodeDone, this, &PhoneLoginWidget::sendVerifyCodeDone, Qt::UniqueConnection);
    dataCenter->getVerifyCodeAsync(phone);

    // 开始倒计时
    timer->start(1000);
}

void PhoneLoginWidget::sendVerifyCodeDone()
{
    Toast::showMessage("验证码已经发送");
}

void PhoneLoginWidget::clickSubmitBtn()
{
    // 输入框获取内容
    const QString& phone = currentPhone;
    const QString& verifyCode = verifyCodeEdit->text();
    if (phone.isEmpty() || verifyCode.isEmpty())
    {
        Toast::showMessage("电话或者验证码不应该为空");
        return;
    }

    // 发送请求
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (isLoginMode) // 登录
    {
        connect(dataCenter, &IM::Model::DataCenter::phoneLoginDone, this, &PhoneLoginWidget::phoneLoginDone, Qt::UniqueConnection);
        dataCenter->phoneLoginAsync(phone, verifyCode);
    }
    else // 注册
    {
        connect(dataCenter, &IM::Model::DataCenter::phoneRegisterDone, this, &PhoneLoginWidget::phoneRegisterDone, Qt::UniqueConnection);
        dataCenter->phoneRegisterAsync(phone, verifyCode);
    }
}

void PhoneLoginWidget::phoneLoginDone(bool ok, const QString& reason)
{
    if (!ok)
    {
        Toast::showMessage("登录失败! " + reason);
        return;
    }

    // 跳转主窗口
    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->show();
    close();
}

void PhoneLoginWidget::phoneRegisterDone(bool ok, const QString &reason)
{
    if (!ok)
    {
        Toast::showMessage("注册失败! " + reason);
        return;
    }
    Toast::showMessage("注册成功!");

    switchMode();
    verifyCodeEdit->clear();
    leftTime = 1;
}

void PhoneLoginWidget::countDown()
{
    if (leftTime <= 1)
    {
        sendVerifyCodeBtn->setEnabled(true);
        sendVerifyCodeBtn->setText("发送验证码");
        timer->stop();
        leftTime = 30;
        return;
    }

    leftTime -= 1;
    sendVerifyCodeBtn->setText(QString::number(leftTime) + " s");
    if (sendVerifyCodeBtn->isEnabled())
        sendVerifyCodeBtn->setEnabled(false);
}

void PhoneLoginWidget::switchMode()
{
    if (isLoginMode)// 切换注册模式
    {
        setWindowTitle("注册");
        titleLabel->setText("注册");
        submitBtn->setText("注册");
        switchModeBtn->setText("登录");
    }
    else // 切换登录模式
    {
        setWindowTitle("登录");
        titleLabel->setText("登录");
        submitBtn->setText("登录");
        switchModeBtn->setText("注册");
    }
    isLoginMode = !isLoginMode;
}
