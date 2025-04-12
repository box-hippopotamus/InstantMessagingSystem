#ifndef GROUPSESSIONDETAILWIDGET_H
#define GROUPSESSIONDETAILWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include <QLineEdit>

#include "sessiondetailwidget.h"
#include "debug.h"

class AvatarItem;

class GroupSessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    GroupSessionDetailWidget(QWidget* parent);

    void initSignalSlot();
    void initData();
    void initMembers(const QString& chatSessionId);

    void addMember(AvatarItem* avatarItem);

private:
    QGridLayout* glayout;
    QLabel* groupNameLabel;
    QLineEdit* groupNameEdit;
    AvatarItem* addBtn;
    QPushButton* modifyBtn;
    QPushButton* exitGroupBtn;

    QString modifyName;

    int total = 0; // 当前总元素个数
};

#endif // GROUPSESSIONDETAILWIDGET_H
