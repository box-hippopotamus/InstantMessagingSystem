#ifndef TOAST_H
#define TOAST_H

#include <QDialog>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

class Toast : public QDialog
{
    Q_OBJECT
public:
    Toast(const QString& text);

    static void showMessage(const QString& text);
};

#endif // TOAST_H
