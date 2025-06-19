#ifndef DATABASESINGLETON_H
#define DATABASESINGLETON_H

#include <QThreadStorage>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QDebug>
#include <QThread>

#define DB_NAME "../../wdb.db"

class DataBaseSingleton;

class DataBaseSingletonDestroyer {
private:
    DataBaseSingleton* p_instance;
public:
    ~DataBaseSingletonDestroyer();
    void initialize(DataBaseSingleton* p);
};

class DataBaseSingleton {
private:
    static DataBaseSingleton* p_instance;
    static DataBaseSingletonDestroyer destroyer;
    QSqlDatabase db;
    QThreadStorage<QString> currentUserLogin;
    QThreadStorage<bool> isAuth;

    DataBaseSingleton();
    DataBaseSingleton(const DataBaseSingleton&) = delete;
    DataBaseSingleton& operator=(DataBaseSingleton&) = delete;
    ~DataBaseSingleton();
    friend class DataBaseSingletonDestroyer;

public:
    static DataBaseSingleton* getInstance();

    // Методы аутентификации
    QString auth(const QString& log, const QString& pass);
    QString reg(const QString& log, const QString& pass);
    QString logout();

    // Методы работы с погодой
    QStringList weather(const QString& city, int days);
    QStringList getClothingRecommendation(double temp, double humidity, const QString& condition);

    // Методы работы с календарем
    QString addEvent(const QString& date, const QString& eventText);
    QStringList getEvents();

    QSqlDatabase& getDatabase() { return db; }

    // Методы работы с избранными городами
    QString addFavoriteCity(const QString& city);
    QString removeFavoriteCity(const QString& city);
    QStringList getFavoriteCities();

    // Методы для админа
    QString setUserRole(const QString& userLogin, const QString& role);  // Назначить роль (только админ)
    QStringList getAllUsers();  // Получить список всех пользователей (только админ)
    QString deleteUser(const QString& userLogin);  // Удалить пользователя (только админ)

    // Проверка роли текущего пользователя
    bool isAdmin() const;
    QString toggleUserBlock(const QString& userLogin);  // Блокировка/разблокировка пользователя
    QString changeAdminPassword(const QString& newPass);  // Смена пароля текущего администратора
    QStringList getAllAdmins();  // Получение списка всех админов
    QString massDeleteUsers(const QStringList& userLogins);  // Массовое удаление пользователей
    void setRequestLimit(const QString& userLogin, int limit);  // Установка лимита запросов
    int getRequestLimit(const QString& userLogin) const;  // Получение лимита запросов

    QStringList getFavoriteCitiesWeather();  // Получить погоду для всех избранных городов
};

#endif // DATABASESINGLETON_H
