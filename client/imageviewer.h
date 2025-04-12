#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QKeyEvent>

class ZoomableImageView : public QLabel
{
    Q_OBJECT

public:
    explicit ZoomableImageView(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    void setScaleFactor(double factor);
    void resetScaleFactor();
    void resetTransform();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QImage originalImage;
    double scaleFactor;
    bool isDragging;
    QPoint lastDragPos;
    QPoint offset;
};

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer();

    void showImage(const QImage &image);
    void loadFromFile(const QString &filePath);
    void loadFromData(const QByteArray &data);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVBoxLayout *layout;
    ZoomableImageView *imageView;
};

#endif // IMAGEVIEWER_H
