#include "messageshowarea.h"
#include "mainwidget.h"

using namespace IM::Model;

MessageShowArea::MessageShowArea()
{
    // 初始化基本属性
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setWidgetResizable(true);

    // 滚动条样式
    static const QString scrollBarStyle =
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: rgba(240, 243, 250, 30);"
        "    width: 8px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(180, 190, 210, 120);"
        "    min-height: 30px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(160, 170, 190, 150);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "    background: none;"
        "}"
        "QScrollBar:horizontal { height: 0px; }";

    verticalScrollBar()->setStyleSheet(scrollBarStyle);
    horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");

    // 容器样式
    setStyleSheet("QScrollArea {"
                  "    border: none;"
                  "    background: white;"
                  "}");

    // 创建 Container
    container = new QWidget();
    container->setStyleSheet("background: white;");
    setWidget(container);

    // container 添加布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);
    container->setLayout(layout);

#if TEST_UI
    UserInfo userInfo;
    userInfo.userId = QString::number(1000);
    userInfo.nickname = "张三";
    userInfo.description = "从今天开始认真敲代码";
    userInfo.avatar = QIcon(":/resource/image/defaultAvatar.png");
    userInfo.phone = "18612345678";
    Message message = Message::makeMessage(MessageType::TEXT_TYPE, "", userInfo,
                                           QString("这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息"
                                                   "这是一条测试消息这是一条测试消息").toUtf8(), "");
    addMessage(false, message);

    for (int i = 1; i <= 30; ++i)
    {
        UserInfo userInfo;
        userInfo.userId = QString::number(1000 + i);
        userInfo.nickname = "张三" + QString::number(i);
        userInfo.description = "从今天开始认真敲代码";
        userInfo.avatar = QIcon(":/resource/image/defaultAvatar.png");
        userInfo.phone = "18612345678";
        Message message = Message::makeMessage(MessageType::TEXT_TYPE, "", userInfo,
                                               (QString("这是一条测试消息") + QString::number(i)).toUtf8(), "");
        addMessage(true, message);
    }
#endif
}

void MessageShowArea::addMessage(bool isLeft, const Message &message)
{
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft, message);
    container->layout()->addWidget(messageItem);
}

void MessageShowArea::addFrontMessage(bool isLeft, const Message &message)
{
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft, message);
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    layout->insertWidget(0, messageItem);
}

void MessageShowArea::clear()
{
    QLayout* layout = container->layout();
    for (int i = layout->count() - 1; i >= 0; --i)
    {
        QLayoutItem* item = layout->takeAt(i);
        if (item != nullptr && item->widget() != nullptr)
            delete item->widget();
    }
}

void MessageShowArea::scrollToEnd()
{
    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [=]()
    {
        // 获取到垂直滚动条的最大值
        int maxValue = verticalScrollBar()->maximum();
        // 设置滚动条的滚动位置
        verticalScrollBar()->setValue(maxValue);

        timer->stop();
        timer->deleteLater();
    });
    timer->start(500);
}

MessageItem::MessageItem(bool isLeft)
    : isLeft(isLeft)
{}

MessageItem *MessageItem::makeMessageItem(bool isLeft, const Message &message)
{
    // 创建对象和布局管理器
    MessageItem* messageItem = new MessageItem(isLeft);
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(10);
    messageItem->setLayout(layout);

    // 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(40, 40);
    avatarBtn->setIconSize(QSize(40, 40));
    avatarBtn->setIcon(message.sender.avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none;}");
    if (isLeft)
        layout->addWidget(avatarBtn, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    else
        layout->addWidget(avatarBtn, 0, 1, 2, 1, Qt::AlignTop | Qt::AlignLeft);

    // 创建名字和时间容器
    QWidget* headerWidget = new QWidget();
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);  // 设置昵称和时间之间的间距

    // 昵称标签
    QLabel* nameLabel = new QLabel(message.sender.nickname);
    nameLabel->setStyleSheet("QLabel { font-size: 12px; color: #555555; font-weight: 500; }");

    // 时间标签
    QLabel* timeLabel = new QLabel(message.time);
    timeLabel->setStyleSheet("QLabel { font-size: 11px; color: #999999; }");

    // 添加小圆点分隔符
    QLabel* dotSeparator = new QLabel("•");
    dotSeparator->setStyleSheet("QLabel { color: #bbbbbb; font-size: 10px; }");

    headerLayout->addWidget(nameLabel);
    headerLayout->addWidget(dotSeparator);
    headerLayout->addWidget(timeLabel);
    headerLayout->addStretch();

    if (isLeft)
        layout->addWidget(headerWidget, 0, 1);
    else
        layout->addWidget(headerWidget, 0, 0, Qt::AlignRight);

    // 创建消息体
    QWidget* contentWidget = nullptr;
    switch (message.messageType)
    {
    case MessageType::TEXT_TYPE:
        contentWidget = makeTextMessageItem(isLeft, message.content);
        break;
    case MessageType::IMAGE_TYPE:
        contentWidget = makeImageMessageItem(isLeft, message.fileId, message.content);
        break;
    case MessageType::FILE_TYPE:
        contentWidget = makeFileMessageItem(isLeft, message);
        break;
    case MessageType::SPEECH_TYPE:
        contentWidget = makeSpeechMessageItem(isLeft, message);
        break;
    default:
        LOG() << "错误的消息类型! messageType=" << (int)message.messageType;
    }

    if (isLeft)
        layout->addWidget(contentWidget, 1, 1);
    else
        layout->addWidget(contentWidget, 1, 0);

    // 连接信号槽, 用户点击头像
    connect(avatarBtn, &QPushButton::clicked, messageItem, [=]() {
        MainWidget* mainWidget = MainWidget::getInstance();
        UserInfoWidget* userInfoWidget = new UserInfoWidget(message.sender, mainWidget);
        userInfoWidget->exec();
    });

    // 当用户修改了昵称, 同步修改此处
    if (!isLeft)
    {
        // 只是针对右侧消息 做下列操作
        DataCenter* dataCenter = DataCenter::getInstance();
        connect(dataCenter, &DataCenter::changeNicknameDone, messageItem, [=]() {
            nameLabel->setText(dataCenter->getMyself()->nickname);
        });

        connect(dataCenter, &DataCenter::changeAvatarDone, messageItem, [=]() {
            UserInfo* myself = dataCenter->getMyself();
            avatarBtn->setIcon(myself->avatar);
        });
    }

    return messageItem;
}

QWidget *MessageItem::makeTextMessageItem(bool isLeft, const QString& text)
{
    MessageContentLabel* messageContentLabel = new MessageContentLabel(text, isLeft, MessageType::TEXT_TYPE, "", QByteArray());
    return messageContentLabel;
}

QWidget *MessageItem::makeImageMessageItem(bool isLeft, const QString& fileId, const QByteArray& content)
{
    MessageImageLabel* messageImageLabel = new MessageImageLabel(fileId, content, isLeft);
    return messageImageLabel;
}

QWidget *MessageItem::makeFileMessageItem(bool isLeft, const Message& message)
{
    MessageContentLabel* messageContentLabel = new MessageContentLabel("[文件] " + message.fileName, isLeft, message.messageType,
                                                                       message.fileId, message.content);
    return messageContentLabel;
}

QWidget *MessageItem::makeSpeechMessageItem(bool isLeft, const Message& message)
{
    MessageContentLabel* messageContentLabel = new MessageContentLabel("[语音]", isLeft, message.messageType,
                                                                       message.fileId, message.content);
    return messageContentLabel;
}

MessageContentLabel::MessageContentLabel(const QString &text,
                                         bool isLeft,
                                         IM::Model::MessageType messageType,
                                         const QString& fileId,
                                         const QByteArray& content)
    : isLeft(isLeft)
    , messageType(messageType)
    , fileId(fileId)
    , content(content)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont font;
    font.setFamily("微软雅黑");
    font.setPixelSize(14);

    label = new QLabel(this);
    label->setText(text);
    label->setFont(font);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setWordWrap(true); // 文本自动换行
    label->setStyleSheet("QLabel { padding: 10px; line-height: 150%; background-color: transparent; }");


    if (messageType == MessageType::TEXT_TYPE)
        return;

    // 针对文件消息, 并且 content 为空的情况下, 通过网络来加载数据
    if (content.isEmpty()) // 文件消息
    {
        DataCenter* dataCenter = DataCenter::getInstance();
        connect(dataCenter, &DataCenter::getSingleFileDone, this, &MessageContentLabel::updateUI);
        dataCenter->getSingleFileAsync(fileId);
    }
    else
    {
        loadContentDone = true;
    }
}

void MessageContentLabel::paintEvent(QPaintEvent *event)
{
    (void) event;

    // 获取到父元素的宽度
    QObject* object = parent();
    if (!object->isWidgetType())
        return;

    QWidget* parent = dynamic_cast<QWidget*>(object);

    // 计算文本需要的宽度和高度
    QFontMetrics metrics(label->font());

    // 先设置一个最大宽度限制
    int maxWidth = parent->width() * 0.6;

    // 先计算文本在不限制宽度的情况下需要的实际宽度
    int textWidth = 0;

    // 检查文本是否包含换行符
    if (label->text().contains('\n'))
    {
        // 如果有换行符，找出最长的一行
        QStringList lines = label->text().split('\n');
        for (const QString &line : lines)
        {
            int lineWidth = metrics.horizontalAdvance(line);
            textWidth = qMax(textWidth, lineWidth);
        }
        // 限制最大宽度
        textWidth = qMin(textWidth, maxWidth - 40); // 减去左右内边距
    }
    else
    {
        // 如果是单行文本，直接计算宽度
        textWidth = metrics.horizontalAdvance(label->text());
        // 限制最大宽度
        textWidth = qMin(textWidth, maxWidth - 40);
    }

    // 设置实际使用的宽度（加上内边距）
    int pwidth = textWidth + 40;

    // 计算文本在给定宽度下需要的高度
    QRect textRect = metrics.boundingRect(QRect(0, 0, pwidth - 40, 0),
                                          Qt::TextWordWrap | Qt::AlignLeft,
                                          label->text());

    // 如果是单行文本且宽度较小，则适应文本宽度
    if (textRect.height() <= metrics.height() && textRect.width() < pwidth - 40)
        pwidth = textRect.width() + 40; // 加上左右内边距

    // 计算实际需要的高度，考虑内边距
    int height = textRect.height() + 20; // 上下各10px内边距

    // 确保最小高度
    height = qMax(height, 40);

    // 为气泡绘制预留更多空间，防止底部截断
    setMinimumHeight(height + 5); // 增加组件自身的最小高度

    // 绘制圆角矩形和箭头
    QPainter painter(this);
    QPainterPath path;
    painter.setRenderHint(QPainter::Antialiasing);

    if (isLeft)
    {
        // 左侧消息样式（接收消息）
        painter.setPen(QPen(QColor(240, 243, 250), 1));  // 浅灰边框
        painter.setBrush(QColor(240, 243, 250));        // 中间栏背景色

        // 绘制圆角矩形
        painter.drawRoundedRect(10, 0, pwidth, height, 6, 6);
        // 绘制箭头
        path.moveTo(10, 15);
        path.lineTo(0, 20);
        path.lineTo(10, 25);
        path.closeSubpath();
        painter.drawPath(path);

        label->setGeometry(10, 0, pwidth, height);
    }
    else
    {
        // 右侧消息样式（发送消息）
        painter.setPen(QPen(QColor(208, 221, 247), 1));
        painter.setBrush(QColor(208, 221, 247));

        // 圆角矩形左侧边的横坐标位置
        int leftPos = width() - pwidth - 10;
        // 圆角矩形右侧边的横坐标位置
        int rightPos = width() - 10;
        // 绘制圆角矩形
        painter.drawRoundedRect(leftPos, 0, pwidth, height, 6, 6);
        // 绘制箭头
        path.moveTo(rightPos, 15);
        path.lineTo(rightPos + 10, 20);
        path.lineTo(rightPos, 25);
        path.closeSubpath();
        painter.drawPath(path);

        label->setGeometry(leftPos, 0, pwidth, height);
    }

    // 重新设置父元素的高度，确保能够容纳消息区域，并留出足够的底部空间
    parent->setFixedHeight(height + 50); // 增加底部间距以防止气泡被截断
}

void MessageContentLabel::mousePressEvent(QMouseEvent *event)
{
    // 实现鼠标点击之后, 触发文件另存为
    if (event->button() != Qt::LeftButton)
        return;

    if (messageType != MessageType::FILE_TYPE
        && messageType != MessageType::SPEECH_TYPE)
        return;

    if (!loadContentDone)
    {
        Toast::showMessage("数据尚未加载成功, 请稍后重试");
        return;
    }

    // 左键按下
    if (messageType == MessageType::FILE_TYPE)
    {
        saveAsFile(content);
    }
    else if (messageType == MessageType::SPEECH_TYPE)
    {
        SoundRecorder* soundRecorder = SoundRecorder::getInstance();
        label->setText("播放中...");
        connect(soundRecorder, &SoundRecorder::soundPlayDone, this, &MessageContentLabel::playDone, Qt::UniqueConnection);
        soundRecorder->startPlay(content);
    }
}

void MessageContentLabel::updateUI(const QString &fileId, const QByteArray &fileContent)
{
    if (fileId != this->fileId)
        return;

    content = fileContent;
    loadContentDone = true;
    update();
}

// void MessageContentLabel::saveAsFile(const QByteArray &content)
// {
//     QString filePath = QFileDialog::getSaveFileName(this, "另存为", QDir::homePath(), "*");
//     if (filePath.isEmpty())
//     {
//         LOG() << "用户取消了文件另存为";
//         return;
//     }
//     IM::writeByteArrayToFile(filePath, content);
// }

void MessageContentLabel::saveAsFile(const QByteArray &content, const QString &defaultFileName)
{
    QString initialPath;
    if (QFileInfo(defaultFileName).isAbsolute())
        initialPath = defaultFileName;
    else
        initialPath = QDir::home().filePath(defaultFileName);

    QString filePath = QFileDialog::getSaveFileName(this, "另存为", initialPath,  "All Files (*)");

    if (filePath.isEmpty())
    {
        LOG() << "用户取消了文件另存为";
        return;
    }

    IM::writeByteArrayToFile(filePath, content);
}

void MessageContentLabel::saveAsFile(const QByteArray &content)
{
    saveAsFile(content, "");
}

void MessageContentLabel::playDone()
{
    if (label->text() == "播放中...")
        label->setText("[语音]");
}

void MessageContentLabel::contextMenuEvent(QContextMenuEvent *event)
{
    (void) event;
    if (messageType != MessageType::SPEECH_TYPE)
    {
        LOG() << "非语音消息暂时不支持右键菜单";
        return;
    }

    QMenu* menu = new QMenu(this);
    QAction* action = menu->addAction("语音转文字");
    menu->setStyleSheet("QMenu { color: rgb(0, 0, 0); }");
    connect(action, &QAction::triggered, this, [=]() {
        DataCenter* dataCenter = DataCenter::getInstance();
        connect(dataCenter, &DataCenter::speechConvertTextDone, this, &MessageContentLabel::speechConvertTextDone, Qt::UniqueConnection);
        dataCenter->speechConvertTextAsync(fileId, content);
    });

    menu->exec(event->globalPos());
    delete menu;
}

void MessageContentLabel::speechConvertTextDone(const QString &fileId, const QString &text)
{
    if (this->fileId != fileId)
        return;

    label->setText("[语音转文字] " + text);
    update();
}

MessageImageLabel::MessageImageLabel(const QString &fileId, const QByteArray &content, bool isLeft)
    : fileId(fileId)
    , content(content)
    , isLeft(isLeft)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    imageBtn = new QPushButton(this);
    imageBtn->setStyleSheet("QPushButton { border: none; }");

    if (content.isEmpty()) // content 为空说明不是普通文本
    {
        DataCenter* dataCenter = DataCenter::getInstance();
        connect(dataCenter, &DataCenter::getSingleFileDone, this, &MessageImageLabel::updateUI);
        dataCenter->getSingleFileAsync(fileId);
    }

    connect(imageBtn, &QPushButton::clicked, this, [this](){
        if (this->content.isEmpty())
        {
            Toast::showMessage("图片尚未加载完成!");
            return;
        }
        ImageViewer *viewer = new ImageViewer();
        viewer->loadFromData(this->content);
    });
}

void MessageImageLabel::updateUI(const QString& fileId, const QByteArray& content)
{
    if (this->fileId != fileId)
        return;

    // 显示图片内容
    this->content = content;

    // 绘制图片到界面
    update();
}

void MessageImageLabel::paintEvent(QPaintEvent *event)
{
    (void) event;
    // 此处显示的图片宽度的上限
    QObject* object = parent();
    if (!object->isWidgetType())
        return;
    QWidget* parent = dynamic_cast<QWidget*>(object);
    int width = parent->width() * 0.6;
    // 加载二进制数据为图片对象
    QImage image;
    if (content.isEmpty())
    {   // 图片未加载
        QByteArray tmpContent = IM::loadFileToByteArray(":/resource/image/image.png");
        image.loadFromData(tmpContent);
    }
    else
    {
        image.loadFromData(content);
    }
    // 图片缩放
    int height = 0;
    if (image.width() > width)
    {
        height = ((double)image.height() / image.width()) * width;
    }
    else
    {
        width = image.width();
        height = image.height();
    }

    // 添加高度限制，确保图片不超过固定大小
    int maxHeight = parent->height() * 0.6; // 设置最大高度为父控件高度的60%
    if (height > maxHeight) {
        height = maxHeight;
        width = ((double)image.width() / image.height()) * height;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    // 使用更好的缩放算法以提高图片质量
    pixmap = pixmap.scaled(QSize(width, height), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    imageBtn->setIconSize(QSize(width, height));
    imageBtn->setIcon(QIcon(pixmap));
    // 给用户名预留位置
    parent->setFixedHeight(height + 50);
    // 确定按钮位置
    if (isLeft)
    {
        imageBtn->setGeometry(10, 0, width, height);
    }
    else
    {
        int leftPos = this->width() - width - 10;
        imageBtn->setGeometry(leftPos, 0, width, height);
    }
}
