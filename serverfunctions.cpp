#include "serverfuncions.h"
#include <QByteArray>
#include <QStringList>

QByteArray parse(QString data_from_client)
{
    QStringList data_from_client_list = data_from_client.split(QLatin1Char('&'));

    if (data_from_client_list.isEmpty()) {
        return "error: empty input";
    }

    QString nameOfFunc = data_from_client_list.takeFirst();

    if (nameOfFunc == "auth") {
        if (data_from_client_list.size() < 2) {
            return "error: insufficient parameters for auth";
        }
        return auth(data_from_client_list.at(0), data_from_client_list.at(1));
    }
    else if (nameOfFunc == "reg") {
        if (data_from_client_list.size() < 3) {
            return "error: insufficient parameters for reg";
        }
        return reg(data_from_client_list.at(0), data_from_client_list.at(1), data_from_client_list.at(2));
    }
    else {
        return "error: unknown function";
    }
}

QByteArray auth(QString log, QString pass)
{
    return "plug";
}

QByteArray reg(QString log, QString pass, QString mail)
{
    return "plug";
}

QByteArray get_stat(QString stat1, QString stat2, QString stat3)
{
    return "plug";
}

QByteArray check_task(QString task1, QString task2, QString task3)
{
    return "plug";
}
