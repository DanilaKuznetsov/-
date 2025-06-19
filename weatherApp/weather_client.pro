QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WeatherClient
TEMPLATE = app

SOURCES += main.cpp \
    loginform.cpp \
    registerform.cpp \
    weatherform.cpp \
    windowmanager.cpp \
    clientapi.cpp \
    serverthread.cpp  # Новый файл для сервера

HEADERS += loginform.h \
    registerform.h \
    weatherform.h \
    windowmanager.h \
    clientapi.h \
    serverthread.h  # Новый файл для сервера

FORMS += loginform.ui \
    registerform.ui \
    weatherform.ui
