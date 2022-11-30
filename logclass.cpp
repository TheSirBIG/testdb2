#include "logclass.h"
#include "errorcodes.h"

void logClass::_threadSlot(int thrID, int errCode, QString *outStrPtr)
{
    //функция на всякий случай, вдруг понадобится
    Q_UNUSED(thrID);
    Q_UNUSED(errCode);
    Q_UNUSED(outStrPtr);
}

bool logClass::_createTable(QString tname, QSqlError* sqlError)
{
    QSqlQuery query(QSqlDatabase::database(dbConnName));
    bool retval = query.exec("create table if not exists " + tname + " (id bigint unsigned auto_increment primary key, dt datetime(3), txt char(100)) engine=innodb");
    if(!retval)
    {
        *sqlError = query.lastError();
        std::cout << "Error creating table " << tname.toStdString() << std::endl;
    }
    return retval;
}

int logClass::write(QString txt)
{
    int retval = errorCodes::CLASS_NO_ERROR;

    int freeThread = getFreeThread();
    if(freeThread == -1)
    {
        retval = errorCodes::CLASS_NO_FREE_THREADS;
    }
    else
    {
        csvThreadArray[freeThread].ready = false;
        csvThreadArray[freeThread].dt = QDateTime::currentDateTime();
        csvThreadArray[freeThread].txt = txt;
        csvThreadArray[freeThread].startWork = true;
    }

    return retval;
}
