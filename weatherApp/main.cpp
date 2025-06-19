#include <QApplication>
#include <QStackedWidget>
#include <QThread>
#include "windowmanager.h"
#include "serverthread.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Создаём и запускаем сервер в отдельном потоке
    ServerThread serverThread;
    serverThread.start();  // Запуск сервера

    // Создаём стек для окна и WindowManager
    QStackedWidget stackedWidget;
    WindowManager windowManager(&stackedWidget);

    // Создаём форму для авторизации
    LoginForm *loginForm = new LoginForm();
    QObject::connect(loginForm, &LoginForm::loginSuccess, &windowManager, &WindowManager::switchToWeatherWindow);

    windowManager.switchToLoginWindow();
    stackedWidget.show();

    // Запуск приложения
    return app.exec();
}
