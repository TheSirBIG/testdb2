#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include "dbwritecsvthread.h"
#include <QDateTime>

class logThread : public DBWriteCSVThread
{
    void _doWork() override;
    void _endWork() override;
    void _saveForLost() override;
    void _prepareQuery() override;

public:
    logThread():DBWriteCSVThread()
    {

    };
    QDateTime dt;
    QString txt;
    QString queryText;
};

#endif // LOGTHREAD_H
