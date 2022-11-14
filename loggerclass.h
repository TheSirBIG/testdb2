#ifndef LOGGERCLASS_H
#define LOGGERCLASS_H

#include <QtSql>
#include <QApplication>

class loggerClass
{
    QSqlDatabase db;
    QSettings* iniFile;
    QString dbUser;
    QString dbPassword;
    QString dbAddress;

public:
    loggerClass(QString iniSectionName);
};

#endif // LOGGERCLASS_H
