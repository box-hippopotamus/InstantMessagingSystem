#include "mainwidget.h"

#include "loginwidget.h"
#include "network/netclient.h"
#include "debug.h"

#include <QApplication>

FILE* output = nullptr;

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    (void) type;
    (void) context;

    QByteArray log = msg.toUtf8();
    fprintf(output, "%s\n", log.constData());
    fflush(output);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if LOG_TO_FILE
    output = fopen("./log.txt", "a");
    qInstallMessageHandler(messageHandler);
#endif

#if TEST_SKIP_LOGIN
    MainWidget* w = MainWidget::getInstance();
    // w->setWindowFlags(Qt::FramelessWindowHint);
    w->show();
#else
    LoginWidget* l = new LoginWidget(nullptr);
    l->show();
#endif

#if TEST_NETWORK
    IM::Model::DataCenter* dataCenter = IM::Model::DataCenter::getInstance();
    dataCenter->ping();
#endif

    return a.exec();
}
