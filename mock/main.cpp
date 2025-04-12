#include "widget.h"
#include "server.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    HttpServer* httpServer = HttpServer::getInstance();
    if(!httpServer->init())
    {
        qDebug() << "HTTP 服务器启动失败!";
        return 1;
    }
    qDebug() << "HTTP 服务器启动成功!";

    WebsocketServer* websocketServer = WebsocketServer::getInstance();
    if (!websocketServer->init()) {
        qDebug() << "websocket 服务器启动失败!";
        return 1;
    }
    qDebug() << "websocket 服务器启动成功!";

    Widget w;
    w.show();
    return a.exec();
}
