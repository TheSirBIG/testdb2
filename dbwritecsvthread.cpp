#include "dbwritecsvthread.h"
#include "errorcodes.h"

void DBWriteCSVThread::run()
{
    while(1)
    {
        if(startWork)
        {
            startWork = false;
            _doWork();
            outStr = "string data = " + QString::number(threadID) + "," + QString::number(errorCodes::THREAD_END_OF_WORK);
            if(!lostCSV)
            {
                emit workEnd(threadID, errorCodes::THREAD_END_OF_WORK /* ,&outStr*/ );
                ready = true;
            }
        }
        else if(mustFinish)
        {
            _endWork();
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
