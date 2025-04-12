#ifndef SESSIONDETAILWIDGET_H
#define SESSIONDETAILWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFontMetrics>
#include <QMessageBox>

#include "debug.h"
#include "model/data.h"
#include "model/datacenter.h"
#include "choosefrienddialog.h"

using IM::Model::UserInfo;

// 表示一个头像 +  一个名字组合控件
class AvatarItem : public QWidget
{
    Q_OBJECT
public:
    AvatarItem(const QIcon& avatar, const QString& name);

    QPushButton* getAvatar();

private:
    QPushButton* avatarBtn;
    QLabel* nameLabel;
};


// 单聊会话详情窗口
class SessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    SessionDetailWidget(QWidget* parent, const UserInfo& userInfo);

    void clickDeleteFriendBtn();

private:
    QPushButton* deleteFriendBtn;

    UserInfo userInfo;
};

#endif // SESSIONDETAILWIDGET_H
