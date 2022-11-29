#include "qwethread.h"

void qweThread::_doWork()
{
    if(lostCSV)
    {
//        std::cout << "qwe into 'lost' thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(2);
    }
    else
    {
        std::cout << "qwe into thread " << QString::number(threadID).toStdString() << std::endl;
        sleep(4);
    }
}

void qweThread::_endWork()
{
    std::cout << "qwe finish state " << QString::number(threadID).toStdString() << std::endl;
}

void qweThread::_saveForLost()
{

}

void qweThread::_prepareQuery()
{

}
