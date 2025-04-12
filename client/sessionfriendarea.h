#ifndef SESSIONFRIENDAREA_H
#define SESSIONFRIENDAREA_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include <QPainter>
#include <QStyleOption>

#include "common.h"
#include "debug.h"


enum class ItemType
{
    SessionItemType,
    FriendItemType,
    ApplyItemType
};

// 列表
class SessionFriendArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit SessionFriendArea(QScrollArea* parent = nullptr);

    // 清空所有 item
    void clear();

    // 如果是 SessionItem, id = chatSessionId
    // 如果是 FriendItem / ApplyItem, id = userId
    void addItem(ItemType itemType,
                 const QString& id,
                 const QIcon& avatar,
                 const QString& name,
                 const QString& text);

    // 通过下标选中 item
    void clickItem(int index);

private:
    QWidget* container;
};

// 列表项
class SessionFriendItem : public QWidget
{
    Q_OBJECT
public:
    SessionFriendItem(QWidget* owner,
                      const QIcon& avatar,
                      const QString& name,
                      const QString& text);


    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    void select();

    virtual void active();

private:
    QWidget* owner; // 指向 SessionFriendArea

    bool selected = false;

protected:
    QLabel* messageLabel;
};

// 会话列表项
class SessionItem : public SessionFriendItem
{
    Q_OBJECT

public:
    SessionItem(QWidget* owner,
                const QString& chatSessionId,
                const QIcon& avatar,
                const QString& name,
                const QString& lastMessage);

    void active() override;

    void updateLastMessage(const QString& chatSessionId);

private:
    QString chatSessionId; // 当前会话 id
    QString text; // 最后一条消息的文本预览
};

// 好友列表项
class FriendItem : public SessionFriendItem
{
    Q_OBJECT

public:
    FriendItem(QWidget* owner,
               const QString& userId,
               const QIcon& avatar,
               const QString& name,
               const QString& description);

    void active() override;

private:
    QString userId; // 好友的用户id
};


// =======================================
// ============== ApplyItem ==============
// =======================================
class ApplyItem : public SessionFriendItem
{
    Q_OBJECT

public:
    ApplyItem(QWidget* owner,
              const QString& userId,
              const QIcon& avatar,
              const QString& name);

    void active() override;

    void acceptFriendApply();
    void rejectFriendApply();

private:
    QString userId; // 申请人的 userId
};

#endif // SESSIONFRIENDAREA_H
