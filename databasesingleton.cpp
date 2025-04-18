#include "databasesingleton.h"

DataBaseSingletonDestroyer::~DataBaseSingletonDestroyer()
{
    delete p_instance;
}

void DataBaseSingletonDestroyer::initialize(DataBaseSingleton * p)
{
    p_instance = p;
}

DataBaseSingleton::DataBaseSingleton()
{
    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbname);

    if(!db.open())
        qDebug()<<db.lastError().text();
}

DataBaseSingleton::~DataBaseSingleton()
{
    db.close();
}

DataBaseSingleton* DataBaseSingleton::getInstance()
{
    if (!p_instance)
    {
        p_instance = new DataBaseSingleton();
        destroyer.initialize(p_instance);
    }
    return p_instance;
}

QString DataBaseSingleton::auth(QString log, QString pass, int id_conn)
{
    QSqlQuery query(this->db);
    query.prepare("SELECT * FROM users WHERE login = :login AND password = :password");
    query.bindValue(":password",pass);
    query.bindValue(":login",log);
    query.exec();
    if(query.size()>0)
    {
        query.prepare("UPDATE users SET id_conn = :id_conn WHERE login = :login AND password = :password");
        query.bindValue(":password", pass);
        query.bindValue(":login", log);
        query.bindValue(":login", id_conn);
        query.exec();
        return "auth+&"+log;
    }
    else
        return "auth error";
}

QString DataBaseSingleton::reg(QString log, QString pass)
{
    QSqlQuery query(this->db);
    query.prepare("SELECT * FROM users WHERE login = :login AND password = :password");
    query.bindValue(":password", pass);
    query.bindValue(":login", log);
    query.exec();
    if(query.size()>0)
        return "you are already registered";
    else
    {
        query.prepare("INSERT INTO users (id, login, password, role) VALUES (:id, :login, :password, user)");
        int userId = query.lastInsertId().toInt();
        query.bindValue(":id", userId);
        query.bindValue(":login", log);
        query.bindValue(":password", pass);
        query.exec();
        return "reg+&" + log;
    }
}

QStringList DataBaseSingleton::weather(QString city)
{
    QStringList weatherlist;
    QSqlQuery query(this->db);
    query.prepare("SELECT * FROM weather WHERE city = :city");
    query.bindValue(":city", city);
    query.exec();
    while (query.next())
    {
        QString temperature = query.value(2).toString();
        weatherlist.append(temperature);
        QString humidity = query.value(3).toString();
        weatherlist.append(humidity);
        QString wind_speed = query.value(4).toString();
        weatherlist.append(wind_speed);
        QString description = query.value(5).toString();
        weatherlist.append(description);
        QString timestamp = query.value(6).toString();
        weatherlist.append(timestamp);
    }
    return weatherlist;
}

QStringList DataBaseSingleton::cities()
{
    QStringList cityList;
    QSqlQuery query(this->db);

    if (!query.exec("SELECT city FROM weather")) {
        qDebug() << "request error:" << query.lastError().text();
        return cityList;
    }

    while (query.next()) {
        QString city = query.value(0).toString();
        cityList.append(city);
    }

    return cityList;
}
