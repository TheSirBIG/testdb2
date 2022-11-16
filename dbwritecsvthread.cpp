#include "dbwritecsvthread.h"

void DBWriteCSVThread::run()
{
    while(1)
    {
        if(startWork)
        {
            startWork = false;
            doWork();
            outStr = "string data = " + QString::number(threadID) + "," + QString::number(DBWriteCSVThread::END_OF_WORK);
            emit workEnd(threadID, DBWriteCSVThread::END_OF_WORK);
//            emit workEnd(threadID, DBWriteCSVThread::END_OF_WORK, &outStr);
        }
        else if(mustFinish)
        {
            endWork();
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


void DBWriteCSVThread::doWork()
{
    if(lostCSV)
    {
        std::cout << "DBWriteCSVThread into 'lost' thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(2);
    }
    else
    {
        std::cout << "DBWriteCSVThread into thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(4);
    }
}

void DBWriteCSVThread::endWork()
{
    std::cout << "DBWriteCSVThread finish state " << QString::number(threadID).toStdString() << std::endl;
}


void qwe::doWork()
{
    if(lostCSV)
    {
        std::cout << "qwe into 'lost' thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(2);
    }
    else
    {
        std::cout << "qwe into thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(4);
    }
}

void qwe::endWork()
{
    std::cout << "qwe finish state " << QString::number(threadID).toStdString() << std::endl;
}

void logThread::doWork()
{
    if(lostCSV)
    {

    }
    else
    {

    }
}

void logThread::endWork()
{
    std::cout << "logThread finish state " << QString::number(threadID).toStdString() << std::endl;
}
