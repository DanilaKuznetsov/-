#ifndef DATABASESINGLETON_H
#define DATABASESINGLETON_H

#include <QThreadStorage>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QDebug>
#include <QThreadStorage>

#define DB_NAME "../../wdb.db"  //Путь к базе данных

class DataBaseSingleton;

class DataBaseSingletonDestroyer {
private:
    DataBaseSingleton * p_instance;
public:
    ~DataBaseSingletonDestroyer();
    void initialize(DataBaseSingleton * p);
};

class DataBaseSingleton {
private:
    static DataBaseSingleton * p_instance;
    static DataBaseSingletonDestroyer destroyer;
    QSqlDatabase db;

    DataBaseSingleton();
    DataBaseSingleton(const DataBaseSingleton&) = delete;
    DataBaseSingleton& operator = (DataBaseSingleton &) = delete;
    ~DataBaseSingleton();
    friend class DataBaseSingletonDestroyer;
    QThreadStorage<bool> isAuth;

public:
    static DataBaseSingleton* getInstance();
    QString auth(QString log, QString pass);
    QString reg(QString log, QString pass);
    QStringList weather(QString city, int days);
};

#endif // DATABASESINGLETON_H
