#include "clientapi.h"
#include <QNetworkRequest>
#include <QUrl>

ClientAPI::ClientAPI(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
}

QByteArray ClientAPI::authenticateUser(const QString &username, const QString &password)
{
    QUrl url("http://your-server-url.com/authenticate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data;
    data.append("{\"username\":\"" + username + "\", \"password\":\"" + password + "\"}");
    
    QNetworkReply *reply = networkManager->post(request, data);
    return reply->readAll();
}

QByteArray ClientAPI::registerUser(const QString &username, const QString &password)
{
    QUrl url("http://your-server-url.com/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data;
    data.append("{\"username\":\"" + username + "\", \"password\":\"" + password + "\"}");
    
    QNetworkReply *reply = networkManager->post(request, data);
    return reply->readAll();
}
