#include "dbwritecsvthread.h"
#include "errorcodes.h"
#include <QtSql>

void DBWriteCSVThread::run()
{
//    while(1)
    while(!mustFinish)
    {
        if(!dbConnected)
        {
//            std::cout << "into dbconnected," << QString::number(threadID).toStdString() << std::endl;
            dbConnected = dbConnect();
        }
        if(startWork && dbConnected)    //запущен цикл записи и есть связь с бд (как минимум - была при последнем обращении)
        {
//            std::cout << "into start&&dbconnected," << QString::number(threadID).toStdString() << std::endl;
            startWork = false;
            _doWork();
            //для lost - ready=false всегда, чтобы не участвовал в поиске свободного потока
            if(!lostCSV)
            {
                ready = true;
            }
            else
            {
                this->msleep(sleepLostTime);
                startWork = true;
                ready = false;
            }
        }
        else if(startWork)  //запущен цикл записи, но связи с бд нет - в обычном надо создать lost
        {
            if(!lostCSV)    //в lost ничего делать не надо, это он как-раз подчищает за другими
            {
//                std::cout << "into start and no dbconnected," << QString::number(threadID).toStdString() << std::endl;
                startWork = false;
                _prepareQuery();
                _saveForLost();
                emit workEnd(threadID, errorCodes::THREAD_SAVED_FOR_LOST/*, &outStr*/);
                ready = true;
            }
            else
            {
                this->msleep(sleepLostTime);
                startWork = true;
                ready = false;
            }
        }
//        else if(mustFinish) //сигнал завершения работы
//        {
//            _endWork();
//            {
//                QSqlDatabase db = QSqlDatabase::database(dbConn,false);
//                if(db.isOpen()) db.close();
//            }
//            QSqlDatabase::removeDatabase(dbConn);
//            break;
//        }
        else //ничего не пришло, делаем паузу... lost - сюда вроде как не доходит!!!, паузу вставил выше
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
    _endWork();
    {
        QSqlDatabase db = QSqlDatabase::database(dbConn,false);
        if(db.isOpen()) db.close();
    }
    QSqlDatabase::removeDatabase(dbConn);
}

bool DBWriteCSVThread::dbConnect()
{
    QSqlDatabase::addDatabase("QMYSQL",dbConn);

    QSqlDatabase db = QSqlDatabase::database(dbConn,false);
    db.setHostName(dbAddress);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    db.setDatabaseName(dbDatabaseName);

    bool retval = db.open();
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
