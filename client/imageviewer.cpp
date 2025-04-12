#include "imageviewer.h"

ZoomableImageView::ZoomableImageView(QWidget *parent)
    : QLabel(parent),
    scaleFactor(1.0),
    isDragging(false),
    offset(0, 0)
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(100, 100);
    setCursor(Qt::OpenHandCursor);
    setMouseTracking(true);
}

void ZoomableImageView::setImage(const QImage &image)
{
    originalImage = image;
    resetScaleFactor();
    resetTransform();
    update();
}

void ZoomableImageView::setScaleFactor(double factor)
{
    // 限制缩放比例在合理范围内
    double newScaleFactor = scaleFactor * factor;
    if (newScaleFactor > 0.1 && newScaleFactor < 20.0)
    {
        scaleFactor = newScaleFactor;
        update();
    }
}

void ZoomableImageView::resetScaleFactor()
{
    scaleFactor = 1.0;
}

void ZoomableImageView::resetTransform()
{
    offset = QPoint(0, 0);
}

void ZoomableImageView::wheelEvent(QWheelEvent *event)
{
    // 如果没有图像，不处理事件
    if (originalImage.isNull())
        return;

    // 获取鼠标在控件中的位置
    QPointF mousePos = event->position();

    // 计算鼠标在图像坐标系中的位置（考虑当前的缩放和偏移）
    QPointF imagePos = (mousePos - offset) / scaleFactor;

    // 根据滚轮方向确定缩放因子
    const double delta = event->angleDelta().y();
    double factor = 1.0;

    // 使用更平滑的缩放步进
    if (delta > 0)
        factor = 1.1;
    else if (delta < 0)
        factor = 0.9;

    // 计算新的缩放系数（并限制在合理范围内）
    double newScaleFactor = scaleFactor * factor;
    if (newScaleFactor < 0.1 || newScaleFactor > 20.0)
    {
        event->accept();
        return;
    }

    // 更新缩放因子
    scaleFactor = newScaleFactor;

    // 计算新的偏移，确保鼠标指向的图像点保持不变
    // 新坐标 = 鼠标位置 - (图像上的点 * 新缩放)
    QPointF newOffset = mousePos - (imagePos * scaleFactor);
    offset = newOffset.toPoint();

    // 更新视图
    update();
    event->accept();
}

void ZoomableImageView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        isDragging = true;
        lastDragPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else
    {
        QLabel::mousePressEvent(event);
    }
}

void ZoomableImageView::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging)
    {
        QPoint delta = event->pos() - lastDragPos;
        offset += delta;
        lastDragPos = event->pos();
        update();
        event->accept();
    }
    else
    {
        QLabel::mouseMoveEvent(event);
    }
}

void ZoomableImageView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragging)
    {
        isDragging = false;
        setCursor(Qt::OpenHandCursor);
        event->accept();
    }
    else
    {
        QLabel::mouseReleaseEvent(event);
    }
}

void ZoomableImageView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (originalImage.isNull())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 计算绘制区域
    QSize scaledSize = originalImage.size() * scaleFactor;
    QPoint center = QPoint(width() / 2, height() / 2);
    QPoint topLeft = center - QPoint(scaledSize.width() / 2, scaledSize.height() / 2) + offset;

    // 绘制图像
    painter.drawImage(QRect(topLeft, scaledSize), originalImage);
}

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    // 设置窗口属性
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("图片查看器");

    // 创建布局
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建图像视图
    imageView = new ZoomableImageView(this);
    layout->addWidget(imageView);
}

ImageViewer::~ImageViewer() {}

void ImageViewer::showImage(const QImage &image)
{
    if (image.isNull())
        return;

    imageView->setImage(image);

    // 调整窗口大小以适应图片，但不超过屏幕的80%
    QScreen *screen = QApplication::primaryScreen();
    QSize screenSize = screen->availableSize();
    int maxWidth = screenSize.width() * 0.8;
    int maxHeight = screenSize.height() * 0.8;

    int width = qMin(image.width() + 40, maxWidth);
    int height = qMin(image.height() + 40, maxHeight);

    resize(width, height);
    show();
}

void ImageViewer::loadFromFile(const QString &filePath)
{
    QImage image(filePath);
    if (!image.isNull())
        showImage(image);
}

void ImageViewer::loadFromData(const QByteArray &data)
{
    QImage image;
    if (image.loadFromData(data))
        showImage(image);
}

void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(event);
}
