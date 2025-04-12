#include "toast.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

Toast::Toast(const QString &text)
{
    // 窗口基本参数
    setFixedSize(320, 80);  // 调整为更紧凑的尺寸
    setWindowTitle("消息通知");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setAttribute(Qt::WA_DeleteOnClose);
    setStyleSheet(
        "QDialog {"
        "   background-color: rgb(240, 243, 250);"  // 使用中间栏底色
        "   border: 1px solid rgb(208, 221, 247);"  // 主蓝灰色边框
        "   border-radius: 8px;"  // 圆角效果
        "}");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);  // 无边框且置顶

    // 窗口位置（居中偏下）
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    move((screenGeometry.width() - width()) / 2,
         screenGeometry.height() - height() - 60);

    // 布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(20, 12, 20, 12);  // 增加内边距
    setLayout(layout);

    // 显示文本
    QLabel* label = new QLabel();
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"  // 更合适的字号
        "   color: rgb(50, 55, 65);"  // 深灰色文字
        "   font-family: Microsoft YaHei;"
        "}");
    label->setText(text);
    layout->addWidget(label);

    // 淡入动画
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    QPropertyAnimation* fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);
    fadeIn->start();

    // 2秒后关闭
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        timer->stop();
        close();
    });
    timer->start(2000);
}

void Toast::showMessage(const QString &text)
{
    Toast* toast = new Toast(text);
    toast->show();
}

