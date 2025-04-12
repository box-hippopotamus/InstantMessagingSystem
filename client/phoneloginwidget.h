#ifndef PHONELOGINWIDGET_H
#define PHONELOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QGridLayout>

#include "model/datacenter.h"
#include "loginwidget.h"
#include "mainwidget.h"

class PhoneLoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PhoneLoginWidget(QWidget *parent = nullptr);

    void sendVerifyCode();
    void sendVerifyCodeDone();

    void clickSubmitBtn();
    void phoneLoginDone(bool ok, const QString& reason);
    void phoneRegisterDone(bool ok, const QString& reason);

    void countDown();
    void switchMode();

private:
    QLineEdit* phoneEdit;
    QPushButton* sendVerifyCodeBtn;
    QLineEdit* verifyCodeEdit;
    QLabel* titleLabel;
    QPushButton* submitBtn;
    QPushButton* switchModeBtn;

    bool isLoginMode = true;
    QString currentPhone = "";
    QTimer* timer;
    int leftTime = 30;
};

#endif // PHONELOGINWIDGET_H
