#include "windowmanager.h"
#include "loginform.h"
#include "weatherform.h"

WindowManager::WindowManager(QStackedWidget *stackedWidget, QWidget *parent)
    : QObject(parent), m_stackedWidget(stackedWidget)
{
    m_loginWindow = new LoginForm();
    m_weatherWindow = new WeatherForm();

    m_stackedWidget->addWidget(m_loginWindow);
    m_stackedWidget->addWidget(m_weatherWindow);
}

void WindowManager::switchToLoginWindow()
{
    m_stackedWidget->setCurrentWidget(m_loginWindow);
}

void WindowManager::switchToWeatherWindow()
{
    m_stackedWidget->setCurrentWidget(m_weatherWindow);
}
