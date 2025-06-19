#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QTcpServer>

class ServerThread : public QThread
{
    Q_OBJECT

public:
    explicit ServerThread(QObject *parent = nullptr);
    void run() override;

private:
    QTcpServer *m_tcpServer;
};

#endif // SERVERTHREAD_H
