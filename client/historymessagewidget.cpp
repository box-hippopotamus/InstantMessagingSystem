#include "historymessagewidget.h"

using namespace IM::Model;
HistoryItem *HistoryItem::makeHistoryItem(const Message &message)
{
    HistoryItem* item = new HistoryItem();
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    item->setStyleSheet("background-color: transparent;");

    QGridLayout* layout = new QGridLayout();
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(10);
    layout->setContentsMargins(0, 8, 10, 8);
    item->setLayout(layout);

    // 头像按钮
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(40, 40);
    avatarBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    avatarBtn->setIconSize(QSize(40, 40));
    avatarBtn->setIcon(message.sender.avatar);
    avatarBtn->setStyleSheet(
        "QPushButton {"
        "   border: 1px solid rgba(208, 221, 247, 0.5);"
        "   border-radius: 2px;"
        "}");

    // 昵称和时间标签
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(message.sender.nickname + " | " + message.time);
    nameLabel->setFixedHeight(20);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setStyleSheet(
        "QLabel {"
        "   color: rgb(90, 100, 120);"
        "   font-size: 13px;"
        "   font-family: Microsoft YaHei;"
        "   font-weight: 400;"
        "}");

    // 消息内容处理
    QWidget* contentWidget = nullptr;
    switch(message.messageType)
    {
    case MessageType::TEXT_TYPE:
    {
        QLabel* label = new QLabel();
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        label->setWordWrap(true);
        label->setText(QString(message.content));
        label->adjustSize();
        label->setStyleSheet(
            "QLabel {"
            "   color: rgb(50, 55, 65);"
            "   font-size: 14px;"
            "}");
        contentWidget = label;
        break;
    }
    case MessageType::IMAGE_TYPE:
        contentWidget = new ImageButton(message.fileId, message.content);
        break;
    case MessageType::FILE_TYPE:
        contentWidget = new FileLabel(message.fileId, message.fileName);
        break;
    case MessageType::SPEECH_TYPE:
        contentWidget = new SpeechLabel(message.fileId);
        break;
    default:
        LOG() << "错误的消息类型! messageType=" << (int)message.messageType;
        break;
    }

    layout->addWidget(avatarBtn, 0, 0, 2, 1);
    layout->addWidget(nameLabel, 0, 1, 1, 1);
    layout->addWidget(contentWidget, 1, 1, 5, 1);

    return item;
}

HistoryMessageWidget::HistoryMessageWidget(QWidget *parent)
    : QDialog(parent)
{
    // 窗口属性
    setFixedSize(600, 600);
    setWindowTitle("历史消息");
    setWindowIcon(QIcon(":/resource/image/logo.png"));
    setStyleSheet(
        "QWidget {"
        "   background-color: rgb(245, 245, 245);"
        "   font-family: Microsoft YaHei;"
        "}");
    setAttribute(Qt::WA_DeleteOnClose);

    // 布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(12);
    layout->setContentsMargins(22, 22, 22, 20);
    setLayout(layout);

    // 单选按钮
    static const QString radioStyle =
        "QRadioButton {"
        "   color: rgb(80, 90, 110);"
        "   font-size: 14px;"
        "   spacing: 8px;"
        "}"
        "QRadioButton::indicator {"
        "   width: 16px;"
        "   height: 16px;"
        "}"
        "QRadioButton::indicator::unchecked {"
        "   border: 1px solid rgb(180, 190, 210);"
        "   border-radius: 2px;"
        "   background-color: rgb(240, 243, 250);"
        "}"
        "QRadioButton::indicator::checked {"
        "   border: 1px solid rgb(208, 221, 247);"
        "   border-radius: 2px;"
        "   background-color: rgb(208, 221, 247);"
        "   image: url(:/resource/image/check.png);"
        "}";

    keyRadioBtn = new QRadioButton("按关键词查询");
    timeRadioBtn = new QRadioButton("按时间查询");
    keyRadioBtn->setStyleSheet(radioStyle);
    timeRadioBtn->setStyleSheet(radioStyle);
    keyRadioBtn->setChecked(true); // 默认按照关键词查询
    layout->addWidget(keyRadioBtn, 0, 0, 1, 2);
    layout->addWidget(timeRadioBtn, 0, 2, 1, 2);

    // 搜索框
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(36);
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchEdit->setPlaceholderText("要搜索的关键词");
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid rgb(208, 221, 247);"
        "   border-radius: 2px;"
        "   background-color: rgb(240, 243, 250);"
        "   font-size: 14px;"
        "   padding: 0 12px;"
        "   color: rgb(50, 55, 65);"
        "}"
        "QLineEdit:focus {"
        "   border-color: rgb(180, 190, 210);"
        "   background-color: white;"
        "}");

    // 搜索按钮
    QPushButton* searchBtn = new QPushButton();
    searchBtn->setFixedSize(36, 36);
    searchBtn->setIconSize(QSize(20, 20));
    searchBtn->setIcon(QIcon(":/resource/image/search.png"));
    searchBtn->setCursor(Qt::PointingHandCursor);
    searchBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(240, 243, 250);"
        "   border-radius: 2px;"
        "   border: 1px solid rgb(208, 221, 247);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgb(208, 221, 247);"
        "}");
    layout->addWidget(searchEdit, 1, 0, 1, 8);
    layout->addWidget(searchBtn, 1, 8, 1, 1);

    // 创建时间相关控件
    QLabel* begTag = new QLabel();
    QLabel* endTag = new QLabel();
    begTag->setText("开始时间");
    endTag->setText("结束时间");
    begTimeEdit = new QDateTimeEdit();
    endTimeEdit = new QDateTimeEdit();

    static const QString timeStyle =
        "QLabel {"
        "   font-size: 13px;"
        "   padding-left: 2px;"
        "}"
        "QDateTimeEdit {"
        "   background: rgb(240, 243, 250);"
        "   border: 1px solid rgb(208, 221, 247);"
        "   border-radius: 2px;"
        "   padding: 4px 8px;"
        "   min-width: 160px;"
        "   font-size: 13px;"
        "}";

    begTag->setStyleSheet(timeStyle);
    endTag->setStyleSheet(timeStyle);
    begTimeEdit->setStyleSheet(timeStyle);
    endTimeEdit->setStyleSheet(timeStyle);
    begTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    endTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    begTimeEdit->setFixedHeight(36);
    endTimeEdit->setFixedHeight(36);

    // 滚动区域
    initScrollArea(layout);

    // 槽函数
    connect(keyRadioBtn, &QRadioButton::clicked, this, [=]() {
        // 隐藏时间相关控件
        layout->removeWidget(begTag);
        layout->removeWidget(begTimeEdit);
        layout->removeWidget(endTag);
        layout->removeWidget(endTimeEdit);
        begTag->hide();
        begTimeEdit->hide();
        endTag->hide();
        endTimeEdit->hide();

        // 关键词搜索框加入布局
        layout->addWidget(searchEdit, 1, 0, 1, 8);
        searchEdit->show();
    });

    connect(timeRadioBtn, &QRadioButton::clicked, this, [=]() {
        // 隐藏关键词搜索框
        layout->removeWidget(searchEdit);
        searchEdit->hide();

        // 显示时间相关控件
        layout->addWidget(begTag, 1, 0, 1, 1);
        layout->addWidget(begTimeEdit, 1, 1, 1, 3);
        layout->addWidget(endTag, 1, 4, 1, 1);
        layout->addWidget(endTimeEdit, 1, 5, 1, 3);
        begTag->show();
        begTimeEdit->show();
        endTag->show();
        endTimeEdit->show();
    });

    connect(searchBtn, &QPushButton::clicked, this, &HistoryMessageWidget::clickSearchBtn);

#if TEST_UI
    for (int i = 0; i < 30; ++i)
    {
        UserInfo sender;
        sender.userId = "";
        sender.nickname = "张三" + QString::number(i);
        sender.avatar = QIcon(":/resource/image/defaultAvatar.png");
        sender.description = "";
        sender.phone = "18612345678";
        Message message = Message::makeMessage(MessageType::TEXT_TYPE, "", sender, QString("息内消息内容消息内容" + QString::number(i)).toUtf8(), "");
        this->addHistoryMessage(message);
    }
#endif
}

void HistoryMessageWidget::addHistoryMessage(const Message &message)
{
    HistoryItem* item = HistoryItem::makeHistoryItem(message);
    container->layout()->addWidget(item);
}

void HistoryMessageWidget::clear()
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    for (int i = layout->count() - 1; i >= 0; --i)
    {
        QWidget* w = layout->itemAt(i)->widget();
        if (w == nullptr)
            continue;

        layout->removeWidget(w);
        w->deleteLater();
    }
}

void HistoryMessageWidget::clickSearchBtn()
{
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    connect(dataCenter, &IM::Model::DataCenter::searchMessageDone, this, &HistoryMessageWidget::clickSearchBtnDone, Qt::UniqueConnection);

    if (keyRadioBtn->isChecked()) // 按照关键词搜索
    {
        const QString& searchKey = searchEdit->text();
        if (searchKey.isEmpty())
            return;

        dataCenter->searchMessageAsync(searchKey);
    }
    else // 按照时间搜索
    {
        auto begTime = begTimeEdit->dateTime();
        auto endTime = endTimeEdit->dateTime();
        if (begTime >= endTime)
        {
            Toast::showMessage("时间非法!");
            return;
        }
        dataCenter->searchMessageByTimeAsync(begTime, endTime);
    }
}

void HistoryMessageWidget::clickSearchBtnDone()
{
    // 获取消息搜索的结果列表
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    QList<Message>* messageResult = dataCenter->getSearchMessageResult();
    if (messageResult == nullptr)
        return;

    // 显示结果
    clear();
    for (const Message& m : *messageResult)
        addHistoryMessage(m);
}

void HistoryMessageWidget::initScrollArea(QGridLayout *layout)
{
    // 滚动区域对象
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);

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

    scrollArea->verticalScrollBar()->setStyleSheet(scrollBarStyle);

    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0; }");
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    // QWidget,要加入的新的内容
    container = new QWidget();
    scrollArea->setWidget(container);

    // 布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(10);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    container->setLayout(vlayout);

    // 滚动区加入到 layout
    layout->addWidget(scrollArea, 2, 0, 1, 9);
}

// 图片消息
ImageButton::ImageButton(const QString &fileId, const QByteArray &content)
    : fileId(fileId)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet("QPushButton { border: none; }");

    if (!content.isEmpty()) // 直接显示到界面上
    {
        updateUI(fileId, content);
    }
    else // 通过网络获取
    {
        DataCenter* dataCenter = DataCenter::getInstance();
        connect(dataCenter, &DataCenter::getSingleFileDone, this, &ImageButton::updateUI);
        dataCenter->getSingleFileAsync(fileId);
    }

    connect(this, &QPushButton::clicked, this, [this](){
        if (this->content.isEmpty())
        {
            Toast::showMessage("图片尚未加载完成!");
            return;
        }
        ImageViewer *viewer = new ImageViewer();
        viewer->loadFromData(this->content);
    });
}

void ImageButton::updateUI(const QString &fileId, const QByteArray &content)
{
    if (this->fileId != fileId)
        return;

    this->content = content;

    QImage image;
    image.loadFromData(content);

    // 此处显示的图片宽度的上限
    QWidget* parent = qobject_cast<QWidget*>(this->parent());
    int width = parent ? parent->width() * 0.6 : 300;
    int height = 0;

    // 图片缩放
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
    int maxHeight = parent ? parent->height() * 0.6 : 300;
    if (height > maxHeight)
    {
        height = maxHeight;
        width = ((double)image.width() / image.height()) * height;
    }

    resize(width, height);
    setIconSize(QSize(width, height));
    QPixmap pixmap = QPixmap::fromImage(image);
    pixmap = pixmap.scaled(QSize(width, height), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setIcon(QIcon(pixmap));
}

// 文件消息
FileLabel::FileLabel(const QString &fileId, const QString &fileName)
    : fileId(fileId)
    , fileName(fileName)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setText("[文件] " + fileName);
    setWordWrap(true);
    adjustSize();
    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // 网络加载数据
    DataCenter* dataCenter = DataCenter::getInstance();
    connect(dataCenter, &DataCenter::getSingleFileDone, this, &FileLabel::getContentDone);
    dataCenter->getSingleFileAsync(this->fileId);
}

void FileLabel::getContentDone(const QString &fileId, const QByteArray &fileContent)
{
    if (fileId != this->fileId)
        return;

    content = fileContent;
    loadDone = true;
}

void FileLabel::mousePressEvent(QMouseEvent *event)
{
    (void) event;
    if (!loadDone)
    {
        Toast::showMessage("文件内容加载中, 请稍后尝试!");
        return;
    }

    // 弹出一个对话框, 让用户来选择当前要保存的位置
    QString filePath = QFileDialog::getSaveFileName(this, "另存为", QDir::homePath(), "*");
    if (filePath.isEmpty())
    {
        LOG() << "用户取消了保存";
        return;
    }

    IM::writeByteArrayToFile(filePath, content);
}

// 语音历史消息
SpeechLabel::SpeechLabel(const QString &fileId)
    : fileId(fileId)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setText("[语音]");
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setWordWrap(true);
    adjustSize();

    DataCenter* dataCenter = DataCenter::getInstance();
    connect(dataCenter, &DataCenter::getSingleFileDone, this, &SpeechLabel::getContentDone);
    dataCenter->getSingleFileAsync(fileId);
}

void SpeechLabel::getContentDone(const QString &fileId, const QByteArray &content)
{
    if (fileId != this->fileId)
        return;

    this->content = content;
    loadDone = true;
}

void SpeechLabel::mousePressEvent(QMouseEvent *event)
{
    (void) event;
    if (!loadDone)
    {
        Toast::showMessage("文件内容加载中, 稍后重试");
        return;
    }

    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->startPlay(this->content);
}
