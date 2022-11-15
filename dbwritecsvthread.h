#ifndef DBWRITECSVTHREAD_H
#define DBWRITECSVTHREAD_H

#include <iostream>
#include <QThread>

//DBWriteCSVThread - общий шаблон
// пример нового класса - qwe
// в каждом наследуемом классе добавлять требуемые переменные, писать свой код в doWork/endWork
//
// signal workEnd   возвращает id потока, код ошибки, и, если требуется, расширенную информацию в qstring
//
// startWork        триггер на запуск обработки в потоке
// mustFinish       триггер на остановку потока, вызывать только перед удалением, т.е. в конце программы
// threadID         id потока, на всякий случай, сгодится, если что, для диагностики ошибок каких-нибудь
// outStr           строка для расширенной информации
//
// doWork           перегружаемая основная функция обработки
// endWork          перегружаемая, вызывается при остановке потока (на всякий случай, вдруг какой код там потребуется)
//


class DBWriteCSVThread : public QThread
{
    Q_OBJECT   //нужен для сигналов workEnd и finished(private в QThread)
    void run() override;
signals:
    void workEnd(int thrID, int errCode, QString* outStrPtr = nullptr);

public:
    bool startWork = false;
    bool mustFinish = false;
    int threadID = 0;
    QString outStr;

    virtual void doWork();
    virtual void endWork();

/* коды, возвращаемые сигналом */
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

