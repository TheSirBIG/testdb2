#ifndef QWETHREAD_H
#define QWETHREAD_H

#include "dbwritecsvthread.h"

class qweThread : public DBWriteCSVThread
{
    void _doWork() override;
    void _endWork() override;
public:
    qweThread():DBWriteCSVThread()
    {

    };
};

#endif // QWETHREAD_H
