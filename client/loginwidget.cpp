#include "loginwidget.h"
#include "phoneloginwidget.h"

using namespace IM::Model;

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget{parent}
{
    // 基本属性
    setFixedSize(320, 380);
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

    // 用户名输入框
    usernameEdit = new QLineEdit();
    usernameEdit->setFixedHeight(35);
    usernameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    usernameEdit->setPlaceholderText("输入用户名");
    usernameEdit->setStyleSheet(editStyle);

    // 密码输入框
    passwordEdit = new QLineEdit();
    passwordEdit->setFixedHeight(35);
    passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    passwordEdit->setPlaceholderText("输入密码");
    passwordEdit->setStyleSheet(editStyle);
    passwordEdit->setEchoMode(QLineEdit::Password);

    // 验证码输入框
    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(35);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    verifyCodeEdit->setPlaceholderText("输入验证码");
    verifyCodeEdit->setStyleSheet(editStyle);

    // 验证码图片的控件
    verifyCodeWidget = new VerifyCodeWidget(this);

    // 登录按钮样式
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

    // 登录按钮
    submitBtn = new QPushButton();
    submitBtn->setText("登录");
    submitBtn->setFixedHeight(35);
    submitBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    submitBtn->setStyleSheet(btnBlueStyle);
    submitBtn->setCursor(Qt::PointingHandCursor);

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

    // 切换到手机号
    phoneModeBtn = new QPushButton();
    phoneModeBtn->setFixedSize(100, 36);
    phoneModeBtn->setText("手机号登录");
    phoneModeBtn->setStyleSheet(btnTransparentBlueStyle);
    phoneModeBtn->setCursor(Qt::PointingHandCursor);

    // 切换模式
    switchModeBtn = new QPushButton();
    switchModeBtn->setFixedSize(100, 36);
    switchModeBtn->setText("注册");
    switchModeBtn->setStyleSheet(btnTransparentBlueStyle);
    switchModeBtn->setCursor(Qt::PointingHandCursor);

    // 添加到布局管理器
    layout->addWidget(titleLabel, 0, 0, 1, 5);
    layout->addWidget(usernameEdit, 1, 0, 1, 5);
    layout->addWidget(passwordEdit, 2, 0, 1, 5);
    layout->addWidget(verifyCodeEdit, 3, 0, 1, 4);
    layout->addWidget(verifyCodeWidget, 3, 4, 1, 1);
    layout->addWidget(submitBtn, 4, 0, 1, 5);
    layout->addWidget(phoneModeBtn, 5, 0, 1, 1);
    layout->addWidget(switchModeBtn, 5, 4, 1, 1);

    // 信号槽
    connect(switchModeBtn, &QPushButton::clicked, this, &LoginWidget::switchMode);

    connect(phoneModeBtn, &QPushButton::clicked, this, [this]() {
        PhoneLoginWidget* phoneLoginWidget = new PhoneLoginWidget(nullptr);
        phoneLoginWidget->show();
        close();
    });

    connect(submitBtn, &QPushButton::clicked, this, &LoginWidget::clickSubmitBtn);
}

void LoginWidget::switchMode()
{
    if (isLoginMode) // 登录模式, 切换到注册模式
    {
        setWindowTitle("注册");
        titleLabel->setText("注册");
        submitBtn->setText("注册");
        phoneModeBtn->setText("手机号注册");
        switchModeBtn->setText("登录");
    }
    else // 注册模式, 切换到登录模式
    {
        setWindowTitle("登录");
        titleLabel->setText("登录");
        submitBtn->setText("登录");
        phoneModeBtn->setText("手机号登录");
        switchModeBtn->setText("注册");
    }
    isLoginMode = !isLoginMode;
}

void LoginWidget::clickSubmitBtn()
{
    const QString& username = usernameEdit->text();
    const QString& password = passwordEdit->text();
    const QString& verifyCode = verifyCodeEdit->text();
    if (username.isEmpty() || password.isEmpty() || verifyCode.isEmpty())
    {
        Toast::showMessage("用户名/密码/验证码不能为空!");
        return;
    }

    // 对比验证码
    if (!verifyCodeWidget->checkVerifyCode(verifyCode))
    {
        Toast::showMessage("验证码错误!");
        verifyCodeWidget->refreshVerifyCode();
        return;
    }

    // 发送网络请求
    DataCenter* dataCenter = DataCenter::getInstance();
    if (isLoginMode)
    {
        connect(dataCenter, &DataCenter::userLoginDone, this, &LoginWidget::userLoginDone);
        dataCenter->userLoginAsync(username, password);
    }
    else
    {
        connect(dataCenter, &DataCenter::userRegisterDone, this, &LoginWidget::userRegisterDone);
        dataCenter->userRegisterAsync(username, password);
    }
}

void LoginWidget::userLoginDone(bool ok, const QString& reason)
{
    if (!ok)
    {
        Toast::showMessage("登录失败! " + reason);
        return;
    }

    // 登录成功, 跳转到主界面
    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->show();
    close();
}

void LoginWidget::userRegisterDone(bool ok, const QString &reason)
{
    if (!ok)
    {
        Toast::showMessage("注册失败! " + reason);
        return;
    }
    Toast::showMessage("注册成功!");

    // 切换到登录界面
    switchMode();

    // 输入框清空
    verifyCodeEdit->clear();

    // 更新验证码
    verifyCodeWidget->refreshVerifyCode();
}
