#include <QCoreApplication>
#include "mytcpserv.h"

int main(int argc, char *argv[]) {
    // Создаём объект приложения, который управляет жизненным циклом программы
    QCoreApplication a(argc, argv);

    // Создаём экземпляр нашего TCP-сервера
    MyTcpServer myserv;

    // Запускаем цикл обработки событий приложения (ожидание и обработка событий)
    return a.exec();
}
