#ifndef MESSAGESHOWAREA_H
#define MESSAGESHOWAREA_H

#include <QScrollArea>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QFileDialog>
#include <QTimer>
#include <QMenu>


#include "userinfowidget.h"
#include "model/data.h"
#include "debug.h"
#include "imageviewer.h"

using IM::Model::Message;

// 消息展示区
class MessageShowArea : public QScrollArea
{
    Q_OBJECT
public:
    MessageShowArea();

    // 修改标题


    // 尾插
    void addMessage(bool isLeft, const Message& message);

    // 头插
    void addFrontMessage(bool isLeft, const Message& message);

    // 清空消息
    void clear();

    // 滚动到末尾
    void scrollToEnd();

private:
    QWidget* container;
};


// 一条消息元素
class MessageItem : public QWidget
{
    Q_OBJECT

public:
    MessageItem(bool isLeft);

    // 工厂
    static MessageItem* makeMessageItem(bool isLeft, const Message& message);

    // 工厂函数
    static QWidget* makeTextMessageItem(bool isLeft, const QString& text);
    static QWidget* makeImageMessageItem(bool isLeft, const QString& fileId, const QByteArray& content);
    static QWidget* makeFileMessageItem(bool isLeft, const Message& message);
    static QWidget* makeSpeechMessageItem(bool isLeft, const Message& message);

private:
    bool isLeft; // 消息的位置
};

// 文本消息
class MessageContentLabel : public QWidget
{
    Q_OBJECT
public:
    // fileName 可以作为 text 的一部分, 传递进来. 不需要单独列一个参数
    MessageContentLabel(const QString& text,
                        bool isLeft,
                        IM::Model::MessageType messageType,
                        const QString& fileId,
                        const QByteArray& content);

    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    void updateUI(const QString& fileId, const QByteArray& fileContent);

    void saveAsFile(const QByteArray &content, const QString &defaultFileName);
    void saveAsFile(const QByteArray& content);

    void playDone();

    void contextMenuEvent(QContextMenuEvent* event) override;
    void speechConvertTextDone(const QString& fileId, const QString& text);

private:
    QLabel* label;
    bool isLeft;

    IM::Model::MessageType messageType;
    QString fileId;
    QString fileName;
    QByteArray content;

    bool loadContentDone = false;
};

// 图片消息
class MessageImageLabel : public QWidget
{
    Q_OBJECT

public:
    MessageImageLabel(const QString& fileId,
                      const QByteArray& content,
                      bool isLeft);

    void updateUI(const QString& fileId, const QByteArray& content);
    void paintEvent(QPaintEvent* event);

private:
    QPushButton* imageBtn;

    QString fileId;  		// 该图片在服务器对应的文件 id.
    QByteArray content;		// 图片的二进制数据
    bool isLeft;
};

#endif // MESSAGESHOWAREA_H
