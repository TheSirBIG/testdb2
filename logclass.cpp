#include "logclass.h"
#include "errorcodes.h"

void logClass::_threadSlot(int thrID, int errCode, QString *outStrPtr)
{
//    std::cout << "logclass slot from thread: " << QString::number(thrID).toStdString() << ", code: " <<
//                 QString::number(errCode).toStdString() << std::endl;
//    if(outStrPtr == nullptr)
//        std::cout << "logclass outstr null pointer" << std::endl;
//    else
//        std::cout << outStrPtr->toStdString() << std::endl;
}

bool logClass::_createTable(QString tname, QSqlError* sqlError)
{
    bool retval;

    std::cout << "logclass _createtable, name = " << tname.toStdString() << std::endl;
    QSqlQuery query(QSqlDatabase::database(dbConnName));
    retval = query.exec("create table if not exists " + tname + " (id bigint unsigned auto_increment primary key, dt datetime(3), txt char(100)) engine=innodb");
    if(!retval)
    {
        *sqlError = query.lastError();
        std::cout << "some error creating table " << tname.toStdString() << std::endl;
    }
    return retval;
}

int logClass::write(QString txt)
{
    int retval = errorCodes::NO_ERROR;

    int freeThread = getFreeThread();
    if(freeThread == -1)
    {
        retval = errorCodes::CLASS_NO_FREE_THREADS;
    }
    else
    {
//  freeThread = 3;
        csvThreadArray[freeThread].ready = false;
        csvThreadArray[freeThread].dt = QDateTime::currentDateTime();
        csvThreadArray[freeThread].txt = txt;
        csvThreadArray[freeThread].startWork = true;
    }

    return retval;
}
