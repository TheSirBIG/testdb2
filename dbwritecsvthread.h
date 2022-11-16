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
// dbConn           имя соединения с бд
//
// doWork           перегружаемая основная функция обработки
// endWork          перегружаемая, вызывается при остановке потока (на всякий случай, вдруг какой код там потребуется)
// lostCSV          индикатор, что занимается "отложенными" файлами
//                  такой делать только один!!!  - в log, скорее всего, не будет такого???
//                  или таки сделать отдельный поток???
// filePath         путь к файлу csv (если такой используется) - в log, скорее всего, не будет такого???
// tableName        таблица для записи
//

// если обнаружилась ошибка с бд - просто сигнал с ошибкой.
//  если это соединение - кто будет устранять? класс dbconnectclass, или программа выше уровнем???
//      вариант - переименовать файл в определенном формате, поток с lostCSV=true смотрит на эти файлы, если находит - то пишет в бд
//                      если ошибка - просто ничего не делать, если нормально - стирать этот файл
//
//
//
//
//
//

class DBWriteCSVThread : public QThread
{
    Q_OBJECT   //нужен для сигналов workEnd и finished(???)(private в QThread)
    void run() override;
signals:
    void workEnd(int thrID, int errCode, QString* outStrPtr = nullptr);

public:
    bool startWork = false;
    bool mustFinish = false;
    int threadID = 0;
    QString outStr;
    QString dbConn;
    bool lostCSV = false;
    QString filePath;
    int sleepTime = 200;
    int sleepLostTime = 1500;
    QString tableName;

    virtual void doWork();
    virtual void endWork();

/* коды, возвращаемые сигналом */
public:
    static const int END_OF_WORK = 0;
    static const int NOT_CONNECTED = 1;
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

