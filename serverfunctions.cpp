#include "serverfunctions.h"
#include "dbsingleton.h"
#include <QDebug>
#include <QTextStream>

// Вспомогательная функция для безопасного преобразования строки в число
// Принимает строку и ссылку на переменную для сохранения результата
// Возвращает true, если преобразование удалось, иначе false
static bool safeStringToInt(const QString& str, int& result) {
    bool ok;                                   // Флаг успешности преобразования
    result = str.toInt(&ok);                  // Пытаемся преобразовать строку в число
    return ok;                                  // Возвращаем результат
}

// Обработка запроса аутентификации
// Принимает список параметров из запроса
// Возвращает QByteArray с результатом аутентификации
static QByteArray handleAuth(const QStringList& params) {
    if (params.size() != 2) {                     // Проверяем, что параметров ровно 2 (логин и пароль)
        return "error: insufficient parameters for auth"; // Если нет - возвращаем ошибку
    }
    QString log = params.at(0);                  // Получаем логин из первого параметра
    QString pass = params.at(1);                 // Получаем пароль из второго параметра
    // Вызываем метод auth у DataBaseSingleton для аутентификации пользователя
    return DataBaseSingleton::getInstance()->auth(log, pass).toUtf8();
}

// Обработка запроса регистрации
// Принимает список параметров из запроса
// Возвращает QByteArray с результатом регистрации
static QByteArray handleReg(const QStringList& params) {
    if (params.size() != 2) {                     // Проверяем, что параметров ровно 2 (логин и пароль)
        return "error: insufficient parameters for reg"; // Если нет - возвращаем ошибку
    }
    QString log = params.at(0);                  // Получаем логин из первого параметра
    QString pass = params.at(1);                 // Получаем пароль из второго параметра
    // Вызываем метод reg у DataBaseSingleton для регистрации пользователя
    return DataBaseSingleton::getInstance()->reg(log, pass).toUtf8();
}

// Обработка запроса погоды
// Принимает список параметров из запроса
// Возвращает QByteArray с данными о погоде или ошибкой
static QByteArray handleWeather(const QStringList& params) {
    if (params.size() != 2) { // Проверяем, что параметров ровно 2 (город и количество дней)
        return "error: insufficient parameters for weather"; // Если нет - возвращаем ошибку
    }
    QString city = params.at(0);      // Получаем город из первого параметра
    QString daysStr = params.at(1);   // Получаем количество дней из второго параметра

    int days;
    // Пытаемся преобразовать количество дней в число, проверяем, что оно положительное и не больше 30
    if (!safeStringToInt(daysStr, days) || days <= 0 || days > 30) {
        // Если преобразование не удалось или число не соответствует условиям - возвращаем ошибку
        return "error: invalid days parameter. Must be a positive integer less than or equal to 30";
    }

    // Вызываем метод weather у DataBaseSingleton для получения данных о погоде
    QStringList weatherData = DataBaseSingleton::getInstance()->weather(city, days);

    // Проверяем, вернул ли метод ошибку
    if (weatherData.isEmpty() || weatherData.at(0) == "error") {
        // Если да - формируем строку ошибки и возвращаем её
        return ("error&" + weatherData.join("&")).toUtf8(); // "error&message"
    } else {
        // Если нет - формируем строку с данными о погоде и возвращаем её
        return ("weather+&" + weatherData.join("&")).toUtf8(); // "weather+&temp&humidity&..."
    }
}

// Функция разбора запроса от клиента
// Принимает строку запроса
// Возвращает QByteArray с результатом обработки запроса
QByteArray parse(QString data_from_client) {
    // Разделяем строку запроса на список параметров, разделитель - символ '&'
    QStringList data_from_client_list = data_from_client.split(QLatin1Char('&'));

    // Проверяем, что список параметров не пустой
    if (data_from_client_list.isEmpty()) {
        qDebug() << "Error: Empty input received";  // Выводим сообщение об ошибке в консоль
        return "error: empty input";               // Возвращаем сообщение об ошибке
    }

    // Получаем имя функции из первого параметра списка
    QString nameOfFunc = data_from_client_list.takeFirst();

    QByteArray result; // Переменная для хранения результата обработки запроса
    // В зависимости от имени функции вызываем соответствующую функцию обработки
    if (nameOfFunc == "auth") {
        result = handleAuth(data_from_client_list); // Вызываем функцию обработки аутентификации
    } else if (nameOfFunc == "reg") {
        result = handleReg(data_from_client_list);  // Вызываем функцию обработки регистрации
    } else if (nameOfFunc == "weather") {
        result = handleWeather(data_from_client_list); // Вызываем функцию обработки погоды
    } else {
        qDebug() << "Error: Unknown function " << nameOfFunc; // Если имя функции неизвестно - выводим сообщение об ошибке в консоль
        result = "error: unknown function";                   // Возвращаем сообщение об ошибке
    }

    // Выводим в консоль результат обработки запроса
    qDebug() << "Result for " << nameOfFunc << ": " << result;
    return result;
}
