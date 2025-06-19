#ifndef CLIENTAPI_H
#define CLIENTAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ClientAPI : public QObject
{
    Q_OBJECT

public:
    explicit ClientAPI(QObject *parent = nullptr);
    QByteArray authenticateUser(const QString &username, const QString &password);
    QByteArray registerUser(const QString &username, const QString &password);

private:
    QNetworkAccessManager *networkManager;
};

#endif // CLIENTAPI_H
