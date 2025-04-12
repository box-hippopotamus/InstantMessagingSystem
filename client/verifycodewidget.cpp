#include "verifycodewidget.h"
#include <QPainter>

#include "common.h"

VerifyCodeWidget::VerifyCodeWidget(QWidget *parent)
    : QWidget(parent)
    , randomGenerator(IM::getTime())
{
    verifyCode = generateVerifyCode();
}

QString VerifyCodeWidget::generateVerifyCode()
{
    QString code;
    for (int i = 0; i < 4; ++i)
    {
        int init = 'A';
        init += randomGenerator.generate() % 26;
        code += static_cast<QChar>(init);
    }
    return code;
}

void VerifyCodeWidget::refreshVerifyCode()
{
    verifyCode = generateVerifyCode();
    update(); // update 触发 paintEvent, 重绘验证码
}

bool VerifyCodeWidget::checkVerifyCode(const QString &verifyCode)
{
    return this->verifyCode.compare(verifyCode, Qt::CaseInsensitive) == 0;
}

void VerifyCodeWidget::paintEvent(QPaintEvent *event)
{
    (void) event;
    const int width = 180;
    const int height = 80;

    QPainter painter(this);
    QPen pen;
    QFont font("楷体", 25, QFont::Bold, true);
    painter.setFont(font);

    // 画点: 添加随机噪点
    for(int i = 0; i < 100; i++)
    {
        pen = QPen(QColor(randomGenerator.generate() % 256, randomGenerator.generate() % 256, randomGenerator.generate() % 256));
        painter.setPen(pen);
        painter.drawPoint(randomGenerator.generate() % width, randomGenerator.generate() % height);
    }

    // 画线: 添加随机干扰线
    for(int i = 0; i < 5; i++)
    {
        pen = QPen(QColor(randomGenerator.generate() % 256, randomGenerator.generate() % 256, randomGenerator.generate() % 256));
        painter.setPen(pen);
        painter.drawLine(randomGenerator.generate() % width, randomGenerator.generate() % height,
                         randomGenerator.generate() % width, randomGenerator.generate() % height);
    }

    // 绘制验证码
    for(int i = 0; i < verifyCode.size(); i++)
    {
        pen = QPen(QColor(randomGenerator.generate() % 255, randomGenerator.generate() % 255, randomGenerator.generate() % 255));
        painter.setPen(pen);
        painter.drawText(5+20*i, randomGenerator.generate() % 10, 30, 30, Qt::AlignCenter, QString(verifyCode[i]));
    }
}

void VerifyCodeWidget::mousePressEvent(QMouseEvent *event)
{
    (void) event;
    refreshVerifyCode();
}

