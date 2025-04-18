#include "mytcpserv.h"
#include <QDebug>
#include <QCoreApplication>
#include <QString>
#include "serverfuncions.h"

MyTcpServer::~MyTcpServer()
{

    mTcpServer->close();
    //server_status=0;
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent){
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 66666)){
        qDebug() << "сервер не запущен :(";
    } else {
        //server_status=1;
        qDebug() << "сервер запущен :)";
    }
}

void MyTcpServer::slotNewConnection(){
    //   if(server_status==1){
    mTcpSocket = mTcpServer->nextPendingConnection();
    mTcpSocket->write("Привет, я эхо!\r\n");
    connect(mTcpSocket, &QTcpSocket::readyRead,this,&MyTcpServer::slotServerRead);
    connect(mTcpSocket,&QTcpSocket::disconnected,this,&MyTcpServer::slotClientDisconnected);
    // }
}

void MyTcpServer::slotServerRead(){
    QString res = "";
    while(mTcpSocket->bytesAvailable()>0)
    {
        QByteArray array = mTcpSocket->readAll();
        res.append(array);
    }
    qDebug()<<res;
    mTcpSocket->write(parse(res));
}

void MyTcpServer::slotClientDisconnected(){
    mTcpSocket->close();
}
