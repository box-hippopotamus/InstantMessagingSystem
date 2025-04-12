#ifndef USERINFOWIDGET_H
#define USERINFOWIDGET_H

#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QMessageBox>

#include "model/data.h"
#include "model/datacenter.h"

using IM::Model::UserInfo;

class UserInfoWidget : public QDialog
{
    Q_OBJECT
public:
    UserInfoWidget(const UserInfo& userInfo, QWidget* parent);
    void initSignalSlot();
    void clickDeleteFriendBtn();
    void clickApplyBtn();

private:
    const UserInfo& userInfo;

    QPushButton* avatarBtn;
    QLabel* idTag;
    QLabel* idLabel;
    QLabel* nameTag;
    QLabel* nameLabel;
    QLabel* phoneTag;
    QLabel* phoneLabel;

    QPushButton* applyBtn;
    QPushButton* sendMessageBtn;
    QPushButton* deleteFriendBtn;
};

#endif // USERINFOWIDGET_H
