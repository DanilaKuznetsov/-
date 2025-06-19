////////#include "serverfunctions.h"
#include "dbsingleton.h"
#include <QDebug>
#include <QTextStream>
#include <QDate>
#include <QThread>

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
    }/////////
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

    //Арслан
    QStringList weatherData = DataBaseSingleton::getInstance()->weather(city, days);

    if (weatherData.isEmpty() || weatherData.at(0) == "error") {
        return ("error&" + weatherData.join("&")).toUtf8();
    } else {
        return ("weather+&" + weatherData.join("&")).toUtf8();
    }
    //Арслан
}

// АА_правка
static QByteArray handleLogout(const QStringList& params) {
    Q_UNUSED(params); // Параметры не требуются
    return DataBaseSingleton::getInstance()->logout().toUtf8();
}

//Кузнецов
// Обработчик добавления нового события
static QByteArray handleAddEvent(const QStringList& params) {
    // 1. Проверка количества параметров (должны быть дата и текст)
    if (params.size() != 2) {
        return "error: expected date and text"; // Ошибка если параметров не 2
    }

    // 2. Проверка формата даты (должен быть YYYY-MM-DD)
    QDate date = QDate::fromString(params[0], "yyyy-MM-dd");
    if (!date.isValid()) {
        return "error: invalid date format (use YYYY-MM-DD)"; // Ошибка неверного формата
    }

    // 3. Проверка текста события:
    //    - Не может быть пустым
    //    - Максимальная длина 255 символов
    if (params[1].isEmpty() || params[1].length() > 255) {
        return "error: event text must be 1-255 characters"; // Ошибка валидации текста
    }

    // 4. Если все проверки пройдены - добавляем событие через DataBaseSingleton
    //    и преобразуем результат в QByteArray для отправки по сети
    return DataBaseSingleton::getInstance()->addEvent(params[0], params[1]).toUtf8();
}

// Обработчик получения списка событий
static QByteArray handleGetEvents(const QStringList& params) {
    // Q_UNUSED указывает что параметры сознательно не используются
    // (нужно чтобы избежать предупреждений компилятора)
    Q_UNUSED(params);

    // 1. Получаем список событий через DataBaseSingleton
    QStringList events = DataBaseSingleton::getInstance()->getEvents();

    // 2. Преобразуем в QByteArray для отправки по сети
    return ("events+&" + events.join(";")).toUtf8();
}

//Мижитдоржиев
// Обработчик добавления города в избранное
static QByteArray handleAddFavoriteCity(const QStringList& params) {
    if (params.size() != 1) {
        return "error: expected city name";
    }

    if (params[0].isEmpty()) {
        return "error: city name cannot be empty";
    }

    return DataBaseSingleton::getInstance()->addFavoriteCity(params[0]).toUtf8();
}

// Обработчик удаления города из избранного
static QByteArray handleRemoveFavoriteCity(const QStringList& params) {
    if (params.size() != 1) {
        return "error: expected city name";
    }

    return DataBaseSingleton::getInstance()->removeFavoriteCity(params[0]).toUtf8();
}

// Обработчик получения списка избранных городов
static QByteArray handleGetFavoriteCities(const QStringList& params) {
    Q_UNUSED(params);
    QStringList cities = DataBaseSingleton::getInstance()->getFavoriteCities();
    return ("favorites+&" + cities.join(";")).toUtf8();
}

// Обработчик назначения роли
static QByteArray handleSetRole(const QStringList& params) {
    if (params.size() != 2) return "error: expected user and role";
    return DataBaseSingleton::getInstance()->setUserRole(params[0], params[1]).toUtf8();
}

// Обработчик получения списка пользователей
static QByteArray handleGetAllUsers(const QStringList& params) {
    Q_UNUSED(params);
    QStringList users = DataBaseSingleton::getInstance()->getAllUsers();
    return ("users+&" + users.join(";")).toUtf8();
}

// Обработчик удаления пользователя
static QByteArray handleDeleteUser(const QStringList& params) {
    if (params.size() != 1) return "error: expected user login";
    return DataBaseSingleton::getInstance()->deleteUser(params[0]).toUtf8();
}

// Обработчик блокировки/разблокировки пользователя
static QByteArray handleToggleBlock(const QStringList& params) {
    if (params.size() != 1) return "error: expected user login";
    return DataBaseSingleton::getInstance()->toggleUserBlock(params[0]).toUtf8();
}

// Обработчик смены пароля админа
static QByteArray handleChangeAdminPass(const QStringList& params) {
    if (params.size() != 1) return "error: expected new password";
    return DataBaseSingleton::getInstance()->changeAdminPassword(params[0]).toUtf8();
}

// Обработчик получения списка админов
static QByteArray handleGetAdmins(const QStringList& params) {
    Q_UNUSED(params);
    QStringList admins = DataBaseSingleton::getInstance()->getAllAdmins();
    return ("admins+&" + admins.join(";")).toUtf8();
}

// Обработчик массового удаления пользователей
static QByteArray handleMassDelete(const QStringList& params) {
    if (params.isEmpty()) return "error: expected at least one user login";
    return DataBaseSingleton::getInstance()->massDeleteUsers(params).toUtf8();
}

// Обработчик установки лимита запросов
static QByteArray handleSetRequestLimit(const QStringList& params) {
    if (params.size() != 2) return "error: expected user and limit";
    int limit;
    if (!safeStringToInt(params[1], limit) || limit < 0) {
        return "error: invalid limit value";
    }
    DataBaseSingleton::getInstance()->setRequestLimit(params[0], limit);
    return "request limit set";
}

// Обработчик получения лимита запросов
static QByteArray handleGetRequestLimit(const QStringList& params) {
    if (params.size() != 1) return "error: expected user login";
    int limit = DataBaseSingleton::getInstance()->getRequestLimit(params[0]);
    return QString::number(limit).toUtf8();
}

//обработчик получения список избранных городов и погоды в них
static QByteArray handleGetFavoriteCitiesWeather(const QStringList& params) {
    Q_UNUSED(params);
    QStringList weatherData = DataBaseSingleton::getInstance()->getFavoriteCitiesWeather();
    return ("favorites_weather+&" + weatherData.join("&")).toUtf8();
}

QByteArray parse(QString data_from_client) {
    QStringList data_from_client_list = data_from_client.split('&');
    if (data_from_client_list.isEmpty()) {
        return "error: empty input";
    }

    QString nameOfFunc = data_from_client_list.takeFirst();
    QByteArray result;

    if (nameOfFunc == "auth") {
        result = handleAuth(data_from_client_list);
    }
    else if (nameOfFunc == "reg") {
        result = handleReg(data_from_client_list);
    }
    else if (nameOfFunc == "weather") {
        result = handleWeather(data_from_client_list);
    }
    else if (nameOfFunc == "logout") {
        result = handleLogout(data_from_client_list);
    }
    else if (nameOfFunc == "add_event") {
        result = handleAddEvent(data_from_client_list);
    }
    else if (nameOfFunc == "get_events") {
        result = handleGetEvents(data_from_client_list);
    }
    else if (nameOfFunc == "add_favorite") {
        result = handleAddFavoriteCity(data_from_client_list);
    }
    else if (nameOfFunc == "remove_favorite") {
        result = handleRemoveFavoriteCity(data_from_client_list);
    }
    else if (nameOfFunc == "get_favorites") {
        result = handleGetFavoriteCities(data_from_client_list);
    }
    else if (nameOfFunc == "set_role") {
        result = handleSetRole(data_from_client_list);
    }
    else if (nameOfFunc == "get_all_users") {
        result = handleGetAllUsers(data_from_client_list);
    }
    else if (nameOfFunc == "delete_user") {
        result = handleDeleteUser(data_from_client_list);
    }
    else if (nameOfFunc == "toggle_block") {
        result = handleToggleBlock(data_from_client_list);
    }
    else if (nameOfFunc == "change_admin_pass") {
        result = handleChangeAdminPass(data_from_client_list);
    }
    else if (nameOfFunc == "get_admins") {
        result = handleGetAdmins(data_from_client_list);
    }
    else if (nameOfFunc == "mass_delete") {
        result = handleMassDelete(data_from_client_list);
    }
    else if (nameOfFunc == "set_request_limit") {
        result = handleSetRequestLimit(data_from_client_list);
    }
    else if (nameOfFunc == "get_request_limit") {
        result = handleGetRequestLimit(data_from_client_list);
    }
    else if (nameOfFunc == "get_favorites_weather") {
        result = handleGetFavoriteCitiesWeather(data_from_client_list);
    }
    else {
        result = "error: unknown function";
    }

    qDebug() << "Command:" << nameOfFunc << "| Result:" << result;
    return result;
}
