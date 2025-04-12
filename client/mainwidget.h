#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "toast.h"
#include "model/data.h"
#include "model/datacenter.h"
#include "messageshowarea.h"
#include "sessionfriendarea.h"
#include "messageeditarea.h"
#include "selfinfowidget.h"
#include "sessiondetailwidget.h"
#include "groupsessiondetailwidget.h"
#include "addfrienddialog.h"


class MainWidget : public QWidget
{
    Q_OBJECT

public:
    static MainWidget* getInstance();

    void initMainWindow();
    void initLeftWindow();
    void initMidWindow();
    void initRightWindow();

    void initSignalSlot();
    void initWebsocket();

    void switchTabToSession();
    void switchTabToFriend();
    void switchTabToApply();

    void loadSessionList();
    void loadFriendList();
    void loadApplyList();

private:
    MainWidget(QWidget *parent = nullptr);

    ~MainWidget();

private:
    inline static MainWidget* instance = nullptr;

    // 左中右三个窗口
    QWidget* windowLeft;
    QWidget* windowMid;
    QWidget* windowRight;

    // ========== 左侧 ==========
    QPushButton* userAvatar; // 用户头像
    QPushButton* sessionTabBtn; // 会话标签页按钮
    QPushButton* friendTabBtn; // 好友标签页按钮
    QPushButton* applyTabBtn; // 好友申请标签页按钮

    // ========== 中间 ==========
    QLineEdit* searchEdit; // 用户搜索框
    QPushButton* addFriendBtn; // 添加好友按钮
    SessionFriendArea* sessionFriendArea;

    // ========== 右侧 ==========
    QLabel* sessionTitleLabel; // 显示会话标题
    QPushButton* extraBtn; // 显示会话详情按钮
    MessageShowArea* messageShowArea; // 消息展示区
    MessageEditArea* messageEditArea; // 消息编辑区

    void updateFriendList();
    void updateChatSessionList();
    void updateApplyList();

public:
    void loadRecentMessage(const QString& chatSessionId);
    void updateRecentMessage(const QString& chatSessionId);

    // 点击好友项之后, 切换到会话列表的总的函数
    void switchSession(const QString& userId);

    MessageShowArea* getMessageShowArea();

private:
    enum class ActiveTab
    {
        SESSION_LIST,
        FRIEND_LIST,
        APPLY_LIST
    };

    ActiveTab activeTab = ActiveTab::SESSION_LIST;
};
#endif // MAINWIDGET_H
