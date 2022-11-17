#include "logthread.h"

void logThread::_doWork()
{
    if(lostCSV)
    {
//        std::cout << "logthread into 'lost' thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(2);
    }
    else
    {
        std::cout << "logthread into thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(4);
    }
}

void logThread::_endWork()
{
    std::cout << "logThread finish state " << QString::number(threadID).toStdString() << std::endl;
}
