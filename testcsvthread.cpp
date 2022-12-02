#include "testcsvthread.h"
#include "errorcodes.h"
#include <QtSql>

void testCsvThread::_doWork()
{
    int retval;
    QString fn;
    QString fn_only;

    if(lostCSV)
    {
        //поиск lost файлов
        //dbConn всегда вида lc002, lost - вида lc0024, фильтр получается lc* - т.к. надо для всех
        QStringList nameFilter(dbConn.left(dbConn.length()-3) + "*");
        QDir dir(filePath);
        QFileInfoList list = dir.entryInfoList( nameFilter, QDir::Files );
        if(!list.isEmpty())
        {
            fn_only = list.first().fileName();
            fn = list.first().absoluteFilePath();
            QFile file(fn);
            setQuery(fn_only);
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
            //сброс флага соединения с бд, пусть переподключится заново, и, если что, выведет ошибку
            dbConnected = false;
            //сохранить файл для lostCSV !!!!
            _saveForLost();
        }
        else
        {
            QFile::remove(filePath+fName);
        }
    }
}

void testCsvThread::_endWork()
{
    std::cout << "testCsvThread finish state " << QString::number(threadID).toStdString() << std::endl;
}

void testCsvThread::_saveForLost()
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
    //теперь переименовать csv файл в это имя
    QFile::rename(filePath+fName, filename);
}

void testCsvThread::_prepareQuery()
{
    //создание файла csv
    //в примере - создается одна строка
    fName = "csv" + dbConn;
    QFile file(filePath+fName);
    QTextStream stream(&file);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QString ss = "0;";
    for(int i=0; i<100; i++)
    {
        ss += QString::number(data[i]);
        if(i==99) ss+="\n"; else ss+=";";
    }
    stream << ss;
    stream.flush();
    file.close();
    //создание запроса к mysql
//    queryText = "LOAD DATA INFILE '" + sqlFilePath + fName + "' INTO TABLE " + tableName + " FIELDS TERMINATED BY ';' LINES TERMINATED BY '\n' SET id=DEFAULT";
    setQuery(fName);
}

void testCsvThread::setQuery(QString fn)
{
//    queryText = "LOAD DATA INFILE '" + sqlFilePath + fName + "' INTO TABLE " + tableName + " FIELDS TERMINATED BY ';' LINES TERMINATED BY '\n' SET id=DEFAULT";
    queryText = "LOAD DATA INFILE '" + sqlFilePath + fn + "' INTO TABLE " + tableName + " FIELDS TERMINATED BY ';' LINES TERMINATED BY '\n' SET id=DEFAULT";
}
