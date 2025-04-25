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

// Инициализация статического указателя на единственный экземпляр класса
DataBaseSingleton* DataBaseSingleton::p_instance = nullptr;
// Объект-уничтожитель для корректного удаления синглтона при завершении программы
DataBaseSingletonDestroyer DataBaseSingleton::destroyer;

// Деструктор уничтожителя - удаляет синглтон
DataBaseSingletonDestroyer::~DataBaseSingletonDestroyer() {
    delete p_instance;
}

// Метод инициализации уничтожителя, связывает синглтон с уничтожителем
void DataBaseSingletonDestroyer::initialize(DataBaseSingleton *p) {
    p_instance = p;
}

// Метод получения единственного экземпляра класса (паттерн Singleton)
DataBaseSingleton* DataBaseSingleton::getInstance() {
    if (!p_instance) {
        p_instance = new DataBaseSingleton();
        destroyer.initialize(p_instance);
    }
    return p_instance;
}

// Конструктор класса - открывает базу данных и создаёт таблицу пользователей, если её нет
DataBaseSingleton::DataBaseSingleton() {
    db = QSqlDatabase::addDatabase("QSQLITE");      // Используем SQLite
    db.setDatabaseName(DB_NAME);                     // Указываем имя файла базы данных

    if (!db.open()) {
        qDebug() << "Error: could not open database";  // Ошибка открытия БД
    } else {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS users (login TEXT PRIMARY KEY, password TEXT, role TEXT)");
        query.exec("DELETE TABLE weather");
    }
}

// Деструктор - закрывает базу данных
DataBaseSingleton::~DataBaseSingleton() {
    db.close();
}

// Метод авторизации пользователя
QString DataBaseSingleton::auth(QString log, QString pass) {
    QSqlQuery query(db);
    // Хэшируем пароль с помощью SHA256
    QByteArray passwordHash = QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex();
    // Подготавливаем запрос на поиск пользователя с таким логином и паролем
    query.prepare("SELECT * FROM users WHERE login = :login AND password = :password");
    query.bindValue(":login", log);
    query.bindValue(":password", passwordHash);
    query.exec();

    if (query.next()) {
        // Если пользователь найден - устанавливаем флаг авторизации для текущего потока
        isAuth.setLocalData(true);
        return "auth ok ";
    } else {
        // Если нет - сбрасываем флаг авторизации
        isAuth.setLocalData(false);
        return "auth error ";
    }
}

// Метод регистрации нового пользователя
QString DataBaseSingleton::reg(QString log, QString pass) {
    QSqlQuery query(db);
    // Проверяем, существует ли уже пользователь с таким логином
    query.prepare("SELECT * FROM users WHERE login = :login");
    query.bindValue(":login", log);
    query.exec();

    if (query.next()) {
        // Если пользователь уже есть - возвращаем ошибку регистрации
        return "reg error ";
    } else {
        // Иначе хэшируем пароль и добавляем нового пользователя в БД с ролью 'user'
        QByteArray passwordHash = QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex();
        query.prepare("INSERT INTO users (login, password, role) VALUES (:login, :password, 'user')");
        query.bindValue(":login", log);
        query.bindValue(":password", passwordHash);
        query.exec();
        return "reg ok ";
    }
}

// Метод получения прогноза погоды для города на заданное количество дней
QStringList DataBaseSingleton::weather(QString city, int days) {
    // Проверяем, авторизован ли пользователь в текущем потоке
    if (!isAuth.hasLocalData() || !isAuth.localData()) {
        qDebug() << "Access denied: user not authorized";
        return QStringList() << "error" << "Access denied";
    }

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

        // Извлекаем среднюю температуру
        QString temperature = QString::number(day["avgtemp_c"].toDouble(), 'f', 1);
        // Извлекаем среднюю влажность
        QString humidity = QString::number(day["avghumidity"].toDouble(), 'f', 1);
        // Извлекаем описание погоды
        QJsonObject condition = day["condition"].toObject();
        QString description = condition["text"].toString();

        // Формируем строку с данными через разделитель '|'
        dayStrings << QString("%1|%2|%3|%4|%5")
                          .arg(city, dateString, temperature, humidity, description);
    }

    // Формируем итоговый ответ, объединяя все дни через ';' и добавляя префикс
    weatherData.append("weather+&" + dayStrings.join(";"));
    return weatherData;
}
