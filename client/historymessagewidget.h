#ifndef HISTORYMESSAGEWIDGET_H
#define HISTORYMESSAGEWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTimeEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QFileDialog>

#include "debug.h"
#include "toast.h"
#include "model/data.h"
#include "soundrecorder.h"
#include "model/datacenter.h"
#include "imageviewer.h"

using IM::Model::Message;

// 一个历史消息元素
class HistoryItem : public QWidget
{
    Q_OBJECT
public:
    HistoryItem() = default;

    static HistoryItem* makeHistoryItem(const Message& message);
};

// 历史消息窗口
class HistoryMessageWidget : public QDialog
{
    Q_OBJECT
public:
    HistoryMessageWidget(QWidget* parent);

    // 在窗口中添加一个历史消息
    void addHistoryMessage(const Message& message);

    // 清空窗口中所有的历史消息
    void clear();

    void clickSearchBtn();
    void clickSearchBtnDone();

private:
    // 持有所有的历史消息结果的容器对象
    QWidget* container;

    QLineEdit* searchEdit;
    QRadioButton* keyRadioBtn;
    QRadioButton* timeRadioBtn;
    QDateTimeEdit* begTimeEdit;
    QDateTimeEdit* endTimeEdit;

    void initScrollArea(QGridLayout* layout);
};

// 图片历史消息
class ImageButton : public QPushButton
{
public:
    ImageButton(const QString& fileId, const QByteArray& content);
    void updateUI(const QString& fileId, const QByteArray& content);

private:
    QString fileId;
    QByteArray content;
};

// 文件历史消息
class FileLabel : public QLabel
{
public:
    FileLabel(const QString& fileId, const QString& fileName);

    void getContentDone(const QString& fileId, const QByteArray& fileContent);

    void mousePressEvent(QMouseEvent* event) override;

private:
    QString fileId;
    QByteArray content;
    QString fileName;
    bool loadDone = false;
};

// 语音历史消息
class SpeechLabel : public QLabel
{
public:
    SpeechLabel(const QString& fileId);

    void getContentDone(const QString& fileId, const QByteArray& content);

    void mousePressEvent(QMouseEvent* event) override;

private:
    QString fileId;
    QByteArray content;
    bool loadDone = false;
};

#endif // HISTORYMESSAGEWIDGET_H
