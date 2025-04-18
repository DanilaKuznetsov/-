#ifndef DATABASESINGLETON_H
#define DATABASESINGLETON_H

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#define dbname "../../wdb.db"

class DataBaseSingleton;

class DataBaseSingletonDestroyer
{
private:
    DataBaseSingleton * p_instance;
public:
    ~DataBaseSingletonDestroyer();
    void initialize(DataBaseSingleton * p);
};


class DataBaseSingleton
{
private:
    static DataBaseSingleton * p_instance;
    static DataBaseSingletonDestroyer destroyer;
    QSqlDatabase db;
protected:
    DataBaseSingleton();
    DataBaseSingleton(const DataBaseSingleton&) = delete;
    DataBaseSingleton& operator = (DataBaseSingleton &) = delete;
    ~DataBaseSingleton();
    friend class DataBaseSingletonDestroyer;
public:
    static DataBaseSingleton* getInstance();
    QString auth(QString, QString, int);
    QString reg(QString, QString);
    QStringList weather(QString);
    QStringList cities();
};

DataBaseSingleton* DataBaseSingleton::p_instance;
DataBaseSingletonDestroyer DataBaseSingleton::destroyer;

#endif // DATABASESINGLETON_H
