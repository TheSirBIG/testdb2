#include "logthread.h"
#include <QtSql>
#include "errorcodes.h"

void logThread::_doWork()
{
    int retval;

    if(lostCSV)
    {
//        std::cout << "logthread into 'lost' thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(2);
    }
    else
    {
        std::cout << "logthread into thread " << QString::number(threadID).toStdString() << std::endl;
//        sleep(4);

        QSqlQuery query(QSqlDatabase::database(dbConn));
        QString qqq;
        qqq = "insert into " + tableName + " (dt,txt) values('" + dt.toString("yyyy-MM-dd HH:mm:ss.zzz") + "','" + txt + "')";
        std::cout << qqq.toStdString() << std::endl;
        retval = query.exec(qqq);
        if(!retval)
        {
            outStr = query.lastError().text();
            std::cout << "text: " << outStr.toStdString() << std::endl;
            std::cout << "some error in query creating or execute" << std::endl;
            emit workEnd(threadID, errorCodes::THREAD_QUERY_ERROR ,&outStr);
            //сброс флага соединения с бд, пусть переподключится заново, и, если что, ошибку выведет, что за проблема
            dbConnected = false;
            //сохранить файл для lostCSV !!!!
            _saveForLost();
        }
    }
}

void logThread::_endWork()
{
    std::cout << "logThread finish state " << QString::number(threadID).toStdString() << std::endl;
}

void logThread::_saveForLost()
{
    //сохранение для обработки в lostCSV
    std::cout << "into saveforlost" << std::endl;
}
