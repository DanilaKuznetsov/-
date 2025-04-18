#ifndef SERVERFUNCIONS_H
#define SERVERFUNCIONS_H
#include <QByteArray>

QByteArray parse(QString data_from_client);
QByteArray auth(QString log, QString pass);
QByteArray reg(QString log, QString pass, QString mail);
QByteArray get_stat(QStringList pers_stat);
QByteArray check_task(QStringList check_data);

#endif // SERVERFUNCIONS_H
