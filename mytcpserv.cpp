#include "mytcpserv.h"
#include <QDebug>
#include <QByteArray>
#include "serverfunctions.h"

// Деструктор класса MyTcpServer
// Закрывает TCP-сервер и отключает всех клиентов
MyTcpServer::~MyTcpServer() {
    mTcpServer->close();
    // Отключаем все сокеты и очищаем список
    for (QTcpSocket *socket : clientSockets) {
        socket->close();
        socket->deleteLater();
    }
    clientSockets.clear();
}

// Конструктор класса MyTcpServer
// Создаёт сервер, настраивает прослушивание порта и подключает сигнал на новое соединение
MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent) {
    mTcpServer = new QTcpServer(this);  // Создаём объект TCP-сервера с родителем для автоматического удаления

    // Подключаем сигнал о новом входящем соединении к слоту обработки
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    // Запускаем сервер на прослушивание всех интерфейсов на порту 1111
    if (!mTcpServer->listen(QHostAddress::Any, 1111)) {
        qDebug() << "server is not started :(";  // Ошибка при запуске сервера
    } else {
        qDebug() << "server started :)";        // Сервер успешно запущен
    }
}

// Слот, вызываемый при новом входящем соединении
void MyTcpServer::slotNewConnection() {
    // Получаем объект сокета для нового клиента
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();

    // Подключаем сигналы сокета к соответствующим слотам
    connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);

    // Добавляем сокет в список подключенных клиентов
    clientSockets.append(clientSocket);

    qDebug() << "New connection from: " << clientSocket->peerAddress().toString();
}

// Слот для чтения данных от клиента
void MyTcpServer::slotServerRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    // Добавляем данные в буфер для этого клиента
    QByteArray &buffer = clientBuffers[clientSocket];
    buffer.append(clientSocket->readAll());

    // Проверяем, есть ли полная команда (заканчивается на \n)
    while (buffer.contains('\n')) {
        int pos = buffer.indexOf('\n');
        QByteArray command = buffer.left(pos).trimmed();
        buffer = buffer.mid(pos + 1);

        if (!command.isEmpty()) {
            QString res = QString::fromUtf8(command);
            qDebug() << "Complete command received:" << res;
            clientSocket->write(parse(res));
        }
    }
}

// Слот, вызываемый при отключении клиента
void MyTcpServer::slotClientDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    qDebug() << "Client disconnected: " << clientSocket->peerAddress().toString();

    clientBuffers.remove(clientSocket); // Очищаем буфер
    clientSocket->close();
    clientSockets.removeOne(clientSocket);
    clientSocket->deleteLater();
}
