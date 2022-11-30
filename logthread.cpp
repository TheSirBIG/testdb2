#include "logthread.h"
#include <QtSql>
#include "errorcodes.h"

void logThread::_doWork()
{
    int retval;
    QString fn;

    if(lostCSV)
    {
        //поиск lost файлов
        QStringList nameFilter(dbConn.left(dbConn.length()-3) + "*");
        QDir dir(filePath);
        QFileInfoList list = dir.entryInfoList( nameFilter, QDir::Files );
        if(!list.isEmpty())
        {
            fn = list.first().absoluteFilePath();
            QFile file(fn);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            queryText = file.readLine();
            file.close();
            QSqlQuery query(QSqlDatabase::database(dbConn));
            retval = query.exec(queryText);
            if(!retval)
            {
                outStr = query.lastError().text();
                emit workEnd(threadID, errorCodes::THREAD_LOST_QUERY_ERROR ,&outStr);
                //сброс флага соединения с бд, пусть переподключится заново, и, если что, ошибку выведет, что за проблема
                dbConnected = false;
            }
            else
            {
                //запись прошла нормально, удалить файл
                QFile::remove(fn);
            }
        }

    }
    else
    {
        QSqlQuery query(QSqlDatabase::database(dbConn));
        _prepareQuery();
        retval = query.exec(queryText);
        if(!retval)
        {
            outStr = query.lastError().text();
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
    //в случае csv - просто переименовать созданный в _prepareQuery файл
    QString prepfilename = filePath+dbConn;
    QString filename;
    for(int i=0; i<INT_MAX; i++)
    {
        filename = prepfilename + QString::number(i);
        if(!QFile::exists(filename)) break;
    }
    //найдено первое свободное имя
    QFile file(filename);
    QTextStream stream(&file);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    stream << queryText << endl;
    stream.flush();
    file.close();
}

void logThread::_prepareQuery()
{
    queryText = "insert into " + tableName + " (dt,txt) values('" + dt.toString("yyyy-MM-dd HH:mm:ss.zzz") + "','" + txt + "')";
}
