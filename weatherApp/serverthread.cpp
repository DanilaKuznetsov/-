#include "serverthread.h"
#include <QDebug>
#include "mytcpserv.h"

ServerThread::ServerThread(QObject *parent)
    : QThread(parent), m_tcpServer(new MyTcpServer())
{
}

void ServerThread::run()
{
    if (m_tcpServer->listen(QHostAddress::Any, 1111)) {
        qDebug() << "Server started in thread!";
    } else {
        qDebug() << "Failed to start server";
    }

    exec();
}
