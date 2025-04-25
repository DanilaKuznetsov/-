#ifndef MYTCPSERV_H
#define MYTCPSERV_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class MyTcpServer : public QObject {
    Q_OBJECT

public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();

private:
    QTcpServer *mTcpServer;
    QList<QTcpSocket*> clientSockets;
};

#endif // MYTCPSERV_H
