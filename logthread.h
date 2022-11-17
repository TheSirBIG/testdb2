#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include "dbwritecsvthread.h"

class logThread : public DBWriteCSVThread
{
    void _doWork() override;
    void _endWork() override;
public:
    logThread():DBWriteCSVThread()
    {

    };
};

#endif // LOGTHREAD_H
