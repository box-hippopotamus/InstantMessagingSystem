#ifndef CHOOSEFRIENDDIALOG_H
#define CHOOSEFRIENDDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include <QPainter>

#include "debug.h"
#include "toast.h"
#include "model/datacenter.h"

// 单个好友项
class ChooseFriendDialog;

class ChooseFriendItem : public QWidget
{
    Q_OBJECT
public:
    ChooseFriendItem(ChooseFriendDialog* owner,
                     const QString& userId,
                     const QIcon& avatar,
                     const QString& name,
                     bool checked,
                     bool readOnly);

    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    const QString& getUserId() const;

    QCheckBox* getCheckBox();

    bool isReadOnly();

private:
    bool isHover = false;
    bool readOnly;

    QCheckBox* checkBox;
    QPushButton* avatarBtn;
    QLabel* nameLabel;
    ChooseFriendDialog* owner; // 哪个 QWidget 持有这个 Item

    QString userId; // Item 对应的 userId
};

// 选择好友窗口
class ChooseFriendDialog : public QDialog
{
    Q_OBJECT
public:
    ChooseFriendDialog(QWidget* parent, const QList<QString>& userIds, bool isSingelSession);

    void initLeft(QHBoxLayout* layout);
    void initRight(QHBoxLayout* layout);
    void clickOkBtn();
    QList<QString> generateMemberList();

    void initData();

    void addFriend(const QString& userId,
                   const QIcon& avatar,
                   const QString& name,
                   bool checked,
                   bool readOnly);

    void addSelectedFriend(const QString& userId,
                           const QIcon& avatar,
                           const QString& name,
                           bool readOnly);

    void deleteSelectedFriend(const QString& userId);

private:
    // 保存左侧全部好友列表的 QWidget
    QWidget* totalContainer;

    // 保存右侧选中好友列表的 QWidget
    QWidget* selectedContainer;

    // 该会话是否是单聊
    bool isSingelSession;

    // 当前会话已有的所有成员
    QList<QString> userIds;
};

#endif // CHOOSEFRIENDDIALOG_H
