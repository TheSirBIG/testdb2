#ifndef TESTCSVTHREAD_H
#define TESTCSVTHREAD_H

#include "dbwritecsvthread.h"


class testCsvThread : public DBWriteCSVThread
{
    void _doWork() override;
    void _endWork() override;
    void _saveForLost() override;
    void _prepareQuery() override;

    void setQuery(QString fn);
public:
    testCsvThread():DBWriteCSVThread()
    {

    };
    double data[100];
};

#endif // TESTCSVTHREAD_H
