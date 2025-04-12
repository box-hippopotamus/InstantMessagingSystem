#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>

#include "debug.h"
#include "model/data.h"
#include "model/datacenter.h"

using IM::Model::UserInfo;

// 一个好友搜索结果项
class FriendResultItem : public QWidget
{
    Q_OBJECT
public:
    FriendResultItem(const UserInfo& userInfo);
    void clickAddBtn();

private:
    const UserInfo& userInfo;

    QPushButton* addBtn;
};

// 好友搜索窗口
class AddFriendDialog : public QDialog
{
    Q_OBJECT
public:
    AddFriendDialog(QWidget* parent);

    // 初始化结果显示区
    void initResultArea();

    // 往窗口中新增一个好友搜索结果
    void addResult(const UserInfo& userInfo);

    // 清空界面上所有的好友结果
    void clear();

    void setSearchKey(const QString searchKey);

    void clickSearchBtn();
    void clickSearchBtnDone();

private:
    QLineEdit* searchEdit;

    QGridLayout* layout; // 整个窗口网格布局
    QWidget* resultContainer; // 搜索好友结果
};

#endif // ADDFRIENDDIALOG_H
