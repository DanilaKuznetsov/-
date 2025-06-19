#include "dbsingleton.h"
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QEventLoop>
#include <QDateTime>
#include <QSqlError>
#include <QThread>
#include <QDebug>
#include <QStandardPaths>

DataBaseSingleton* DataBaseSingleton::p_instance = nullptr;
DataBaseSingletonDestroyer DataBaseSingleton::destroyer;


DataBaseSingletonDestroyer::~DataBaseSingletonDestroyer() {
    delete p_instance;
}

void DataBaseSingletonDestroyer::initialize(DataBaseSingleton* p) {
    p_instance = p;
}

//Кузнецов
DataBaseSingleton::DataBaseSingleton() {
    // 1. Указываем, что будем использовать SQLite в качестве СУБД
    db = QSqlDatabase::addDatabase("QSQLITE");

    // 2. Устанавливаем путь к файлу базы данных
    db.setDatabaseName(DB_NAME);

    // 3. Пытаемся открыть подключение к базе данных
    if (!db.open()) {
        // Если не удалось открыть - выводим ошибку и завершаем работу конструктора
        qCritical() << "Database opening error:" << db.lastError().text();
        return;
    }

    // 4. Создаем объект для выполнения SQL-запросов
    QSqlQuery query(db);

    // 5. Включаем поддержку внешних ключей (необходимо для ON DELETE CASCADE)
    query.exec("PRAGMA foreign_keys = ON");

    // 6. Создаем таблицу пользователей, если она не существует:
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "login TEXT PRIMARY KEY, "
               "password TEXT NOT NULL, "
               "role TEXT NOT NULL DEFAULT 'user', "
               "request_limit INTEGER NOT NULL DEFAULT 50)");

    // 7. Создаем таблицу событий, если она не существует:
    query.exec("CREATE TABLE IF NOT EXISTS events ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_login TEXT NOT NULL, "
               "event_date TEXT NOT NULL, "
               "event_text TEXT NOT NULL, "
               "FOREIGN KEY(user_login) REFERENCES users(login) ON DELETE CASCADE)");

    // 8. Создаем индекс для поля user_login для ускорения поиска событий пользователя
    query.exec("CREATE INDEX IF NOT EXISTS idx_events_user ON events(user_login)");
    //Мижитдоржиев
    // 9. Создаем таблицу избранных городов, если она не существует:
    query.exec("CREATE TABLE IF NOT EXISTS favorite_cities ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_login TEXT NOT NULL, "
               "city_name TEXT NOT NULL, "
               "UNIQUE(user_login, city_name), "
               "FOREIGN KEY(user_login) REFERENCES users(login) ON DELETE CASCADE)");

    // 10. Создаем индекс для ускорения поиска избранных городов пользователя
    query.exec("CREATE INDEX IF NOT EXISTS idx_favorite_cities_user ON favorite_cities(user_login)");
    //Мижитдоржиев
}
//Кузнецов

// Деструктор - закрывает базу данных
DataBaseSingleton::~DataBaseSingleton() {
    db.close();
}

DataBaseSingleton* DataBaseSingleton::getInstance() {
    if (!p_instance) {
        p_instance = new DataBaseSingleton();
        destroyer.initialize(p_instance);
    }
    return p_instance;
}

// Метод авторизации пользователя
QString DataBaseSingleton::auth(const QString& log, const QString& pass) {
    QSqlQuery query(db);
    QByteArray passwordHash = QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex();

    query.prepare("SELECT role FROM users WHERE login = :login AND password = :password");
    query.bindValue(":login", log);
    query.bindValue(":password", passwordHash);

    if (!query.exec()) {
        return "auth error: " + query.lastError().text();
    }

    if (query.next()) {
        QString role = query.value(0).toString(); // Получаем роль пользователя
        if (role == "blocked") {
            return "auth error: account blocked";  // Заблокированные не могут войти!
        }
        isAuth.setLocalData(true);
        currentUserLogin.setLocalData(log);
        return "auth ok";
    }
    return "auth error: invalid credentials";
}

QString DataBaseSingleton::reg(const QString& log, const QString& pass) {
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT login FROM users WHERE login = :login");
    checkQuery.bindValue(":login", log);

    if (!checkQuery.exec() || checkQuery.next()) {
        return "reg error: user exists";
    }

    QSqlQuery query(db);
    QByteArray passwordHash = QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex();

    query.prepare("INSERT INTO users (login, password, request_limit) VALUES (:login, :password, 5)");
    query.bindValue(":login", log);
    query.bindValue(":password", passwordHash);

    return query.exec() ? "reg ok" : "reg error: " + query.lastError().text();
}

// Функция для выхода из пользователя
QString DataBaseSingleton::logout() {
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return "error: not authorized";
    }
    isAuth.setLocalData(false);
    currentUserLogin.setLocalData(QString());
    return "success: logged out";
}

QStringList DataBaseSingleton::weather(const QString& city, int days) {
    // Проверяем, авторизован ли пользователь в текущем потоке
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        qDebug() << "Access denied: user not authorized";
        return QStringList() << "error" << "Access denied";
    }

    // Проверка лимита запросов
    QString login = currentUserLogin.localData();
    int limit = getRequestLimit(login);

    if (limit <= 0) {
        qDebug() << "Request limit exceeded for user:" << login;
        return QStringList() << "error" << "Request limit exceeded";
    }

    // Уменьшаем лимит на 1
    setRequestLimit(login, limit - 1);

    QStringList weatherData;  // Список для хранения результата
    QString apiKey = "05cac7cb35ab4e95958184915252404";  // Ключ API для weatherapi.com

    // Формируем URL запроса к API с параметрами: ключ, город, количество дней
    QString urlString = QString("http://api.weatherapi.com/v1/forecast.json?key=%1&q=%2&days=%3")
                            .arg(apiKey).arg(city).arg(days);

    QNetworkAccessManager manager;       // Менеджер сетевых запросов
    QUrl url(urlString);                 // URL запроса
    QNetworkRequest netRequest;          // Запрос
    netRequest.setUrl(url);              // Устанавливаем URL в запрос
    QNetworkReply *reply = manager.get(netRequest);  // Отправляем GET-запрос

    QEventLoop loop;  // Цикл ожидания завершения запроса
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();      // Ждём завершения запроса

    // Проверяем, не было ли ошибок сети
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        weatherData << "error" << "Could not retrieve weather data";
        reply->deleteLater();
        return weatherData;
    }

    QByteArray responseData = reply->readAll();  // Читаем ответ
    reply->deleteLater();

    // Парсим JSON из ответа
    QJsonParseError error;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData, &error);

    // Проверяем ошибки парсинга JSON
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
        weatherData << "error" << "Could not parse weather data";
        return weatherData;
    }

    // Получаем объект JSON
    QJsonObject jsonObject = jsonResponse.object();
    // Извлекаем объект с прогнозом
    QJsonObject forecast = jsonObject["forecast"].toObject();
    // Извлекаем массив дней прогноза
    QJsonArray forecastday = forecast["forecastday"].toArray();

    QStringList dayStrings;  // Список строк с данными по каждому дню

    // Берём минимум из запрошенного количества дней и доступных в ответе
    int count = qMin(days, forecastday.size());
    for (int i = 0; i < count; ++i) {
        QJsonObject dayData = forecastday[i].toObject();  // Данные по дню
        QString dateString = dayData["date"].toString();  // Дата
        QJsonObject day = dayData["day"].toObject();      // Объект с погодными параметрами

        double temperature = day["avgtemp_c"].toDouble();
        double humidity = day["avghumidity"].toDouble();
        QJsonObject condition = day["condition"].toObject();
        QString description = condition["text"].toString();

        // Получаем рекомендации по одежде
        QStringList clothes = getClothingRecommendation(temperature, humidity, description);

        // Формируем строку с данными через разделитель '|'
        dayStrings << QString("%1|%2|%3|%4|%5|%6")
                          .arg(city,
                               dateString,
                               QString::number(temperature, 'f', 1),
                               QString::number(humidity, 'f', 1),
                               description,
                               clothes.join(","));
    }

    // Формируем итоговый ответ, объединяя все дни через ';' и добавляя префикс
    weatherData.append("weather+&" + dayStrings.join(";"));
    return weatherData;
}

QStringList DataBaseSingleton::getClothingRecommendation(double temp, double humidity, const QString& condition) {
    QStringList recommendations;

    // Рекомендации по температуре
    if (temp < 0) {
        recommendations << "Термобельё" << "Пуховик" << "Шапка-ушанка" << "Варежки" << "Тёплая обувь";
    }
    else if (temp < 10) {
        recommendations << "Утеплённое пальто" << "Шапка" << "Шарф" << "Перчатки";
    }
    else if (temp < 18) {
        recommendations << "Ветровка" << "Джинсы" << "Лёгкий шарф";
    }
    else {
        recommendations << "Футболка" << "Шорты/Юбка" << "Головной убор от солнца";
    }

    // Дополнительные рекомендации по осадкам
    if (condition.contains("rain", Qt::CaseInsensitive)) {
        recommendations << "Водонепроницаемая куртка" << "Резиновые сапоги" << "Зонт";
    }
    else if (condition.contains("snow", Qt::CaseInsensitive)) {
        recommendations << "Снегозащитные ботинки" << "Непромокаемые варежки";
    }
    else if (condition.contains("sunny", Qt::CaseInsensitive)) {
        recommendations << "Солнцезащитные очки" << "Солнцезащитный крем";
    }

    // Рекомендации по влажности
    if (humidity > 80) {
        recommendations << "Дышащая одежда" << "Влагозащитный чехол для электроники";
    }

    return recommendations;
}

//Кузнецов
// Добавление нового события в календарь
QString DataBaseSingleton::addEvent(const QString& date, const QString& eventText) {
    // 1. Проверка авторизации
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return "error: not authorized";
    }

    // 2. Проверка формата даты (более точная)
    QDate eventDate = QDate::fromString(date, "yyyy-MM-dd");
    if (!eventDate.isValid()) {
        qDebug() << "Invalid date string:" << date;
        return "error: invalid date format (use YYYY-MM-DD)";
    }

    // 3. Проверка что дата не в прошлом (опционально)
    if (eventDate < QDate::currentDate()) {
        return "error: date cannot be in the past";
    }

    // 4. Проверка текста события
    if (eventText.isEmpty()) {
        return "error: event text cannot be empty";
    }
    if (eventText.length() > 255) {
        return "error: event text too long (max 255 characters)";
    }

    // 5. Проверка наличия логина пользователя
    if (!currentUserLogin.hasLocalData()) {
        return "error: user login not available";
    }

    // 6. Вставка события в базу данных
    QSqlQuery query(db);
    query.prepare("INSERT INTO events (user_login, event_date, event_text) "
                  "VALUES (:login, :date, :text)");
    query.bindValue(":login", currentUserLogin.localData());
    query.bindValue(":date", date);
    query.bindValue(":text", eventText);

    if (!query.exec()) {
        qDebug() << "Database error:" << query.lastError().text();
        return "error: database error";
    }

    return "event ok";
}

// Получение списка событий пользователя
QStringList DataBaseSingleton::getEvents() {
    QStringList events; // Список для хранения результатов

    // 1. Проверка авторизации пользователя
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return {"error: not authorized"}; // Возвращаем ошибку если не авторизован
    }

    // 2. Проверка наличия логина пользователя
    if (!currentUserLogin.hasLocalData()) {
        return {"error: user login not available"}; // Ошибка если логин не сохранен
    }

    // 3. Подготавливаем SQL-запрос для выборки событий:
    //    - Выбираем только дату и описание
    //    - Только для текущего пользователя
    //    - Сортировка по дате
    QSqlQuery query(db);
    query.prepare("SELECT event_date, event_text FROM events "
                  "WHERE user_login = :login "
                  "ORDER BY event_date");
    query.bindValue(":login", currentUserLogin.localData());

    // 4. Выполняем запрос
    if (query.exec()) {
        // 5. Обрабатываем результаты:
        //    - Для каждой строки результата формируем строку "дата|описание"
        while (query.next()) {
            events << QString("%1|%2").arg(
                query.value(0).toString(), // Дата события
                query.value(1).toString()  // Описание события
                );
        }
    }

    // 6. Возвращаем результат:
    //    - Если событий нет - возвращаем "no events"
    //    - Иначе - список событий в формате "дата|описание"
    return events.isEmpty() ? QStringList{"no events"} : events;
}
//Кузнецов

//Мижитдоржиев
// Добавление города в избранное
QString DataBaseSingleton::addFavoriteCity(const QString& city) {
    // Проверка авторизации
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return "error: not authorized";
    }

    // Проверка наличия логина пользователя
    if (!currentUserLogin.hasLocalData()) {
        return "error: user login not available";
    }

    // Проверка что город не пустой
    if (city.isEmpty()) {
        return "error: city name cannot be empty";
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO favorite_cities (user_login, city_name) "
                  "VALUES (:login, :city)");
    query.bindValue(":login", currentUserLogin.localData());
    query.bindValue(":city", city);

    if (!query.exec()) {
        if (query.lastError().text().contains("UNIQUE constraint failed")) {
            return "error: city already in favorites";
        }
        return "error: database error - " + query.lastError().text();
    }

    return "favorite ok";
}

// Удаление города из избранного
QString DataBaseSingleton::removeFavoriteCity(const QString& city) {
    // Проверка авторизации
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return "error: not authorized";
    }

    // Проверка наличия логина пользователя
    if (!currentUserLogin.hasLocalData()) {
        return "error: user login not available";
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM favorite_cities "
                  "WHERE user_login = :login AND city_name = :city");
    query.bindValue(":login", currentUserLogin.localData());
    query.bindValue(":city", city);

    if (!query.exec()) {
        return "error: database error - " + query.lastError().text();
    }

    if (query.numRowsAffected() == 0) {
        return "error: city not found in favorites";
    }

    return "favorite removed";
}

// Получение списка избранных городов
QStringList DataBaseSingleton::getFavoriteCities() {
    QStringList cities;

    // Проверка авторизации
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return {"error: not authorized"};
    }

    // Проверка наличия логина пользователя
    if (!currentUserLogin.hasLocalData()) {
        return {"error: user login not available"};
    }

    QSqlQuery query(db);
    query.prepare("SELECT city_name FROM favorite_cities "
                  "WHERE user_login = :login "
                  "ORDER BY city_name");
    query.bindValue(":login", currentUserLogin.localData());

    if (query.exec()) {
        while (query.next()) {
            cities << query.value(0).toString();
        }
    }

    return cities.isEmpty() ? QStringList{"no favorite cities"} : cities;
}

// Проверка, является ли текущий пользователь админом
bool DataBaseSingleton::isAdmin() const {
    if (!isAuth.localData()) return false;

    QSqlQuery query(db);
    query.prepare("SELECT role FROM users WHERE login = :login");
    query.bindValue(":login", currentUserLogin.localData());

    if (query.exec() && query.next()) {
        return query.value(0).toString() == "admin";
    }
    return false;
}

// Назначение роли пользователю (только для админа)
QString DataBaseSingleton::setUserRole(const QString& userLogin, const QString& role) {
    if (!isAdmin()) return "error: permission denied";
    if (role != "user" && role != "admin") return "error: invalid role";

    QSqlQuery query(db);
    query.prepare("UPDATE users SET role = :role WHERE login = :login");
    query.bindValue(":role", role);
    query.bindValue(":login", userLogin);

    return query.exec() ? "role updated" : "error: " + query.lastError().text();
}

// Получить список всех пользователей (только админ)
QStringList DataBaseSingleton::getAllUsers() {
    QStringList users;
    if (!isAdmin()) return {"error: permission denied"};

    QSqlQuery query(db);
    query.prepare("SELECT login, role FROM users");

    if (query.exec()) {
        while (query.next()) {
            users << query.value(0).toString() + "|" + query.value(1).toString();
        }
    }
    return users.isEmpty() ? QStringList{"no users"} : users;
}

// Удаление пользователя (только админ)
QString DataBaseSingleton::deleteUser(const QString& userLogin) {
    if (!isAdmin()) return "error: permission denied";

    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE login = :login");
    query.bindValue(":login", userLogin);

    return query.exec() ? "user deleted" : "error: " + query.lastError().text();
}

// Блокировка/разблокировка пользователя
QString DataBaseSingleton::toggleUserBlock(const QString& userLogin) {
    if (!isAdmin()) return "error: permission denied";

    QSqlQuery query(db);
    query.prepare("SELECT role FROM users WHERE login = :login");
    query.bindValue(":login", userLogin);

    if (!query.exec() || !query.next()) {
        return "error: user not found";
    }

    QString currentRole = query.value(0).toString();
    QString newRole = (currentRole == "blocked") ? "user" : "blocked";

    query.prepare("UPDATE users SET role = :role WHERE login = :login");
    query.bindValue(":role", newRole);
    query.bindValue(":login", userLogin);

    return query.exec() ? QString("user %1").arg(newRole == "blocked" ? "blocked" : "unblocked")
                        : "error: " + query.lastError().text();
}

// Смена пароля текущего администратора
QString DataBaseSingleton::changeAdminPassword(const QString& newPass) {
    if (!isAdmin()) return "error: permission denied";
    if (newPass.length() < 6) return "error: password too short (min 6 characters)";

    QByteArray passwordHash = QCryptographicHash::hash(newPass.toUtf8(), QCryptographicHash::Sha256).toHex();
    QSqlQuery query(db);
    query.prepare("UPDATE users SET password = :password WHERE login = :login");
    query.bindValue(":password", passwordHash);
    query.bindValue(":login", currentUserLogin.localData());

    return query.exec() ? "password changed" : "error: " + query.lastError().text();
}

// Получение списка всех админов
QStringList DataBaseSingleton::getAllAdmins() {
    QStringList admins;
    if (!isAdmin()) return {"error: permission denied"};

    QSqlQuery query(db);
    query.prepare("SELECT login FROM users WHERE role = 'admin'");

    if (query.exec()) {
        while (query.next()) {
            admins << query.value(0).toString();
        }
    }
    return admins.isEmpty() ? QStringList{"no admins"} : admins;
}

// Массовое удаление пользователей
QString DataBaseSingleton::massDeleteUsers(const QStringList& userLogins) {
    if (!isAdmin()) return "error: permission denied";
    if (userLogins.isEmpty()) return "error: no users specified";

    db.transaction();
    QSqlQuery query(db);
    int deletedCount = 0;

    foreach (const QString& login, userLogins) {
        if (login == currentUserLogin.localData()) continue;

        query.prepare("DELETE FROM users WHERE login = :login AND role != 'admin'");
        query.bindValue(":login", login);
        if (query.exec()) {
            deletedCount += query.numRowsAffected();
        }
    }

    if (db.commit()) {
        return QString("%1 users deleted").arg(deletedCount);
    } else {
        db.rollback();
        return "error: transaction failed";
    }
}

void DataBaseSingleton::setRequestLimit(const QString& userLogin, int limit) {
    QSqlQuery query(db);
    query.prepare("UPDATE users SET request_limit = :limit WHERE login = :login");
    query.bindValue(":limit", limit);
    query.bindValue(":login", userLogin);
    if (!query.exec()) {
        qDebug() << "Failed to set request limit:" << query.lastError().text();
    }
    db.commit(); // Явное подтверждение изменений
}


int DataBaseSingleton::getRequestLimit(const QString& userLogin) const {
    QSqlQuery query(db);
    query.prepare("SELECT request_limit FROM users WHERE login = :login");
    query.bindValue(":login", userLogin);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 50; // Значение по умолчанию, если запрос не удался
}

QStringList DataBaseSingleton::getFavoriteCitiesWeather() {
    QStringList result;

    // 1. Проверка авторизации
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        return {"error: not authorized"};
    }

    // 2. Получаем список избранных городов
    QStringList cities = getFavoriteCities();
    if (cities.isEmpty() || cities.first().startsWith("error")) {
        return cities; // Возвращаем ошибку или пустой список
    }

    // 3. Для каждого города получаем погоду (по умолчанию на 1 день)
    foreach (const QString& city, cities) {
        QStringList weatherData = weather(city, 1);
        if (!weatherData.isEmpty() && !weatherData.first().startsWith("error")) {
            // Добавляем данные в формате: город|дата|температура|влажность|описание|одежда
            result << weatherData.join(";");
        }
    }

    return result.isEmpty() ? QStringList{"no weather data for favorite cities"} : result;
}
