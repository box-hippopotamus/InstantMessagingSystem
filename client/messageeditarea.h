#ifndef MESSAGEEDITAREA_H
#define MESSAGEEDITAREA_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QFileDialog>

#include "model/data.h"
#include "model/datacenter.h"
#include "toast.h"
#include "historymessagewidget.h"
#include "soundrecorder.h"

class MessageEditArea : public QWidget
{
    Q_OBJECT
public:
    explicit MessageEditArea(QWidget *parent = nullptr);

    void initSignalSlot();
    void sendTextMessage();
    void addSelfMessage(IM::Model::MessageType messageType, const QByteArray& content, const QString& extraInfo);
    void addOtherMessage(const IM::Model::Message& message);

    void clickSendImageBtn();
    void clickSendFileBtn();

    void soundRecordPressed();
    void soundRecordReleased();
    void sendSpeech(const QString& path);

private:
    QPushButton* sendImageBtn;
    QPushButton* sendFileBtn;
    QPushButton* sendSpeechBtn;
    QPushButton* showHistoryBtn;
    QPlainTextEdit* textEdit;
    QPushButton* sendTextBtn;
    QLabel* tipLabel;

signals:
};

#endif // MESSAGEEDITAREA_H
