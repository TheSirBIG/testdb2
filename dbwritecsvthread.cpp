#include "dbwritecsvthread.h"
#include "errorcodes.h"
#include <QtSql>

void DBWriteCSVThread::run()
{
    bool dbc;
//    int oldSleepTime = sleepTime;
//    int oldSleepLostTime = sleepLostTime;

    while(1)
    {
        if(!dbConnected)
        {
            std::cout << "into dbconnected," << QString::number(threadID).toStdString() << std::endl;
            dbc = dbConnect();
            if(!dbc)
            {
//                oldSleepTime = sleepTime;
//                oldSleepLostTime = sleepLostTime;
//                sleepTime = sleepLostTime = 5000;
                dbConnected = false;
            }
            else
            {
//                sleepTime = oldSleepTime;
//                sleepLostTime = oldSleepLostTime;
                dbConnected = true;
            }
        }
        if(startWork && dbConnected)
        {
            std::cout << "into start&&dbconnected," << QString::number(threadID).toStdString() << std::endl;
            startWork = false;
            _doWork();
            if(!lostCSV)
            {
                //outStr = "string data = " + QString::number(threadID) + "," + QString::number(errorCodes::THREAD_END_OF_WORK);
                //строчку ниже - вроде как и не надо, только для отладки
                //emit workEnd(threadID, errorCodes::THREAD_END_OF_WORK/*, &outStr*/);
                ready = true;
            }
        }
        else if(startWork)
        {
            std::cout << "into start and no dbconnected," << QString::number(threadID).toStdString() << std::endl;
            startWork = false;
            _saveForLost();
            emit workEnd(threadID, errorCodes::THREAD_SAVED_FOR_LOST/*, &outStr*/);
            ready = true;
        }
        else if(mustFinish)
        {
            std::cout << "into mustfinish," << QString::number(threadID).toStdString() << std::endl;
            _endWork();
            {
                QSqlDatabase db = QSqlDatabase::database(dbConn,false);
                if(db.isOpen()) db.close();
            }
            QSqlDatabase::removeDatabase(dbConn);
            break;
        }
        else
        {
            if(lostCSV)
            {
                this->msleep(sleepLostTime);
                startWork = true;
            } else
            {
                this->msleep(sleepTime);
            };
        }
    }
}

//bool DBWriteCSVThread::dbConnect(QString dbAddress, QString dbDatabase, QString dbUser, QString dbPassword)
bool DBWriteCSVThread::dbConnect()
{
    bool retval;

    QSqlDatabase::addDatabase("QMYSQL",dbConn);

    QSqlDatabase db = QSqlDatabase::database(dbConn,false);
    db.setHostName(dbAddress);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    db.setDatabaseName(dbDatabaseName);

    retval = db.open();
    if(!retval)
    {
        QSqlError err = db.lastError();
        outStr = QString::number(err.type()) + ";" + err.text();
        emit workEnd(threadID, errorCodes::THREAD_DATABASE_OPEN, &outStr);
        dbConnected = false;
    }
    else
    {
        dbConnected = true;
    }

    return retval;
}
