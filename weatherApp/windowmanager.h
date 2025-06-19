#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>

class WindowManager : public QObject
{
    Q_OBJECT

public:
    explicit WindowManager(QStackedWidget *stackedWidget, QWidget *parent = nullptr);

    void switchToLoginWindow();
    void switchToWeatherWindow();

private:
    QStackedWidget *m_stackedWidget;
    QWidget *m_loginWindow;
    QWidget *m_weatherWindow;
};

#endif // WINDOWMANAGER_H
