#ifndef DBWRITECSVTHREAD_H
#define DBWRITECSVTHREAD_H

#include <iostream>
#include <QThread>

//DBWriteCSVThread - общий шаблон
//пример нового класса - qwe
//doWork, endWork - в каждом классе делать свои
// добавлять требуемые переменные
class DBWriteCSVThread : public QThread
{
    Q_OBJECT   //нужен для сигналов workEnd и finished(private в QThread)
    void run() override;
signals:
    void workEnd(int thrID, int errCode);

public:
    bool startWork = false;
    int errorCode = 0;
    bool mustFinish = false;
    int threadID = 0;

    virtual void doWork();
    virtual void endWork();

public:
    static const int END_OF_WORK = 0;
};

class qwe : public DBWriteCSVThread
{
    void doWork() override;
    void endWork() override;
};

class logThread : public DBWriteCSVThread
{
    void doWork() override;
    void endWork() override;
};

#endif // DBWRITECSVTHREAD_H

