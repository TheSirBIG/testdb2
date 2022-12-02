#include "testcsvclass.h"
#include "errorcodes.h"

void testCsvClass::_threadSlot(int thrID, int errCode, QString *outStrPtr)
{
    //функция на всякий случай, вдруг понадобится
    Q_UNUSED(thrID);
    Q_UNUSED(errCode);
    Q_UNUSED(outStrPtr);
}

bool testCsvClass::_createTable(QString tname, QSqlError* sqlError)
{
    QSqlQuery query(QSqlDatabase::database(dbConnName));
    QString qtxt = "create table if not exists " + tname + " (id bigint unsigned auto_increment primary key,";
    for(int i=0; i<100; i++)
    {
        qtxt += "vval";
        qtxt += QString::number(i);
        qtxt += " double";
        if(i!=99) qtxt +=",";
    }
    qtxt += ") engine=innodb";
    bool retval = query.exec(qtxt);
    if(!retval)
    {
        *sqlError = query.lastError();
        std::cout << "Error creating table " << tname.toStdString() << std::endl;
    }
    return retval;
}

int testCsvClass::write(double data[100])
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
        for(int i=0; i<100; i++)
            csvThreadArray[freeThread].data[i] = data[i];
        csvThreadArray[freeThread].startWork = true;
    }

    return retval;
}
