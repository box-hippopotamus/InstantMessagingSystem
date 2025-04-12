#include "messageeditarea.h"
#include "mainwidget.h"

MessageEditArea::MessageEditArea(QWidget *parent)
    : QWidget{parent}
{
    setFixedHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 垂直方向布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(5);
    vlayout->setContentsMargins(15, 5, 15, 10);
    setLayout(vlayout);

    // 水平方向布局管理器
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(8);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    vlayout->addLayout(hlayout);

    // 把上方的四个按钮, 创建好并添加到水平布局中
    static const QString btnStyle = "QPushButton {"
                       "    background-color: rgba(240, 243, 250, 50);"
                       "    border: none;"
                       "    border-radius: 4px;"
                       "}"
                       "QPushButton:hover {"
                       "    background-color: rgba(200, 210, 230, 80);"
                       "}"
                       "QPushButton:pressed {"
                       "    background-color: rgba(180, 190, 210, 100);"
                       "}";
    static const QSize btnSize(36, 36);
    static const QSize iconSize(22, 22);

    sendImageBtn = new QPushButton();
    sendImageBtn->setFixedSize(btnSize);
    sendImageBtn->setIconSize(iconSize);
    sendImageBtn->setIcon(QIcon(":/resource/image/image.png"));
    sendImageBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(sendImageBtn);

    sendFileBtn = new QPushButton();
    sendFileBtn->setFixedSize(btnSize);
    sendFileBtn->setIconSize(iconSize);
    sendFileBtn->setIcon(QIcon(":/resource/image/file.png"));
    sendFileBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(sendFileBtn);

    sendSpeechBtn = new QPushButton();
    sendSpeechBtn->setFixedSize(btnSize);
    sendSpeechBtn->setIconSize(iconSize);
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound.png"));
    sendSpeechBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(sendSpeechBtn);

    showHistoryBtn = new QPushButton();
    showHistoryBtn->setFixedSize(btnSize);
    showHistoryBtn->setIconSize(iconSize);
    showHistoryBtn->setIcon(QIcon(":/resource/image/history.png"));
    showHistoryBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(showHistoryBtn);

    // 多行编辑框
    textEdit = new QPlainTextEdit();
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textEdit->setStyleSheet(
        "QPlainTextEdit {"
        "    border: none;"
        "    font-size: 14px;"
        "}"
        "QScrollBar:vertical {"
        "    width: 6px;"
        "    background: transparent;"
        "    margin: 2px 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(180, 190, 210, 150);"
        "    min-height: 30px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(160, 170, 190, 180);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "    background: none;"
        "}"
        );
    vlayout->addWidget(textEdit);

    // 提示 "录制中" QLabel
    tipLabel = new QLabel();
    tipLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tipLabel->setText("录音中...");
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setFont(QFont("微软雅黑", 18, 500));
    tipLabel->setStyleSheet("color: rgb(100, 100, 100);");
    vlayout->addWidget(tipLabel);
    tipLabel->hide();

    // 添加发送文本消息按钮
    sendTextBtn = new QPushButton();
    sendTextBtn->setText("发送");
    sendTextBtn->setFixedSize(100, 36);
    static const QString style =
        "QPushButton {"
        "    font-size: 14px;"
        "    color: white;"
        "    border: none;"
        "    background-color: rgb(100, 160, 220);"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(90, 150, 210);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(80, 140, 200);"
        "}";
    sendTextBtn->setStyleSheet(style);
    vlayout->addWidget(sendTextBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    // 初始化信号槽
    initSignalSlot();
}

void MessageEditArea::initSignalSlot()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();

    // 历史消息
    connect(showHistoryBtn, &QPushButton::clicked, this, [=](){
        if (dataCenter->getCurrentChatSessionId().isEmpty())
            return;

        HistoryMessageWidget* historyMessageWidget = new HistoryMessageWidget(this);
        historyMessageWidget->show();
    });

    // 发送文本消息
    connect(sendTextBtn, &QPushButton::clicked, this, &MessageEditArea::sendTextMessage);
    connect(dataCenter, &IM::Model::DataCenter::sendMessageDone, this, &MessageEditArea::addSelfMessage);

    // 收到消息
    connect(dataCenter, &IM::Model::DataCenter::receiveMessageDone, this, &MessageEditArea::addOtherMessage);

    // 发送图片
    connect(sendImageBtn, &QPushButton::clicked, this, &MessageEditArea::clickSendImageBtn);

    // 发送文件
    connect(sendFileBtn, &QPushButton::clicked, this, &MessageEditArea::clickSendFileBtn);

    // 发送语音
    connect(sendSpeechBtn, &QPushButton::pressed, this, &MessageEditArea::soundRecordPressed);
    connect(sendSpeechBtn, &QPushButton::released, this, &MessageEditArea::soundRecordReleased);
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    connect(soundRecorder, &SoundRecorder::soundRecordDone, this, &MessageEditArea::sendSpeech);
}

void MessageEditArea::sendTextMessage()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getCurrentChatSessionId().isEmpty())
    {
        LOG() << "当前未选中任何会话, 禁止发送消息!";
        Toast::showMessage("当前未选中会话, 禁止发送消息!");
        return;
    }

    // 获取到输入框的内容
    const QString& content = textEdit->toPlainText();
    if (content.isEmpty() || content.trimmed().isEmpty())
    {
        LOG() << "输入框为空";
        Toast::showMessage("输入框为空");
        return;
    }

    textEdit->setPlainText(""); // 清空输入框已有内容
    dataCenter->sendTextMessageAsync(dataCenter->getCurrentChatSessionId(), content);
}

void MessageEditArea::addSelfMessage(IM::Model::MessageType messageType, const QByteArray &content, const QString &extraInfo)
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    const QString& currentChatSessionId = dataCenter->getCurrentChatSessionId();

    // 构造消息对象
    Message message = Message::makeMessage(messageType, currentChatSessionId, *dataCenter->getMyself(), content, extraInfo);
    dataCenter->addMessage(message);

    // 显示消息
    MainWidget* mainWidget = MainWidget::getInstance();
    MessageShowArea* messageShowArea = mainWidget->getMessageShowArea();
    messageShowArea->addMessage(false, message);

    messageShowArea->scrollToEnd();
    emit dataCenter->updateLastMessage(currentChatSessionId);
}

void MessageEditArea::addOtherMessage(const IM::Model::Message &message)
{
    MainWidget* mainWidget = MainWidget::getInstance();
    MessageShowArea* messageShowArea = mainWidget->getMessageShowArea();

    messageShowArea->addMessage(true, message);
    messageShowArea->scrollToEnd();
    Toast::showMessage("收到新消息!");
}

void MessageEditArea::clickSendImageBtn()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getCurrentChatSessionId().isEmpty())
    {
        Toast::showMessage("您尚未选择任何会话, 不能发送图片!");
        return;
    }

    // 弹出文件对话框
    QString filter = "Image Files (*.png *.jpg *.jpeg)";
    QString imagePath = QFileDialog::getOpenFileName(this, "选择图片", QDir::homePath(), filter);
    if (imagePath.isEmpty())
    {
        LOG() << "用户取消选择图片";
        return;
    }

    // 读取图片
    QByteArray imageContent = IM::loadFileToByteArray(imagePath);

    // 发送请求
    dataCenter->sendImageMessageAsync(dataCenter->getCurrentChatSessionId(), imageContent);
}

void MessageEditArea::clickSendFileBtn()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getCurrentChatSessionId().isEmpty())
    {
        Toast::showMessage("您尚未选择任何会话, 不能发送文件!");
        return;
    }

    // 选择文件.
    QString filter = "*";
    QString path = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), filter);
    if (path.isEmpty())
    {
        LOG() << "用户取消选择文件";
        return;
    }

    // 读取文件
    QByteArray content = IM::loadFileToByteArray(path);

    // 传输文件
    QFileInfo fileInfo(path);
    const QString& fileName = fileInfo.fileName();

    // 发送消息
    dataCenter->sendFileMessageAsync(dataCenter->getCurrentChatSessionId(), fileName, content);
}

void MessageEditArea::soundRecordPressed()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getCurrentChatSessionId().isEmpty())
    {
        LOG() << "未选中任何会话, 不能发送语音消息";
        return;
    }

    // 切换语音按钮的图标
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound_active.png"));

    // 开始录音
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->startRecord();

    tipLabel->show();
    textEdit->hide();
}

void MessageEditArea::soundRecordReleased()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    if (dataCenter->getCurrentChatSessionId().isEmpty())
    {
        LOG() << "未选中任何会话, 不能发送语音消息";
        return;
    }

    // 切换语音按钮的图标
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound.png"));

    // 停止录音
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->stopRecord();

    tipLabel->hide();
    textEdit->show();
}

void MessageEditArea::sendSpeech(const QString &path)
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QByteArray content = IM::loadFileToByteArray(path);
    if (content.isEmpty())
    {
        LOG() << "语音文件加载失败";
        return;
    }

    dataCenter->sendSpeechMessageAsync(dataCenter->getCurrentChatSessionId(), content);
}
