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
// ready            готовность потока к работе
// lostCSV          индикатор, что занимается "отложенными" файлами
//                  при установке флаг ready делать false!!!
//      update: устанавливаю этот флаг для последнего потока
// threadID         id(номер) потока, на всякий случай, сгодится, если что, для диагностики ошибок каких-нибудь
// outStr           строка для расширенной информации
// dbConn           имя соединения с бд
// filePath         путь, где будут файлы csv
// sleepTime        пауза для обычного потока
// sleepLostTime    пауза для потока lostCSV
// tableName        имя таблицы для записи
//
// _doWork           перегружаемая основная функция обработки
// _endWork          перегружаемая, вызывается при остановке потока (на всякий случай, вдруг какой код там потребуется)
//

// если обнаружилась ошибка с бд - вызвать сигнал с ошибкой, переименовать файл для 'lost'
//      если ошибка соединения - кто будет устранять? класс dbconnectclass, или программа выше уровнем???


class DBWriteCSVThread : public QThread
{
    Q_OBJECT   //нужен для сигналов workEnd и finished(private в QThread)
    void run() override;
signals:
    void workEnd(int thrID, int errCode, QString* outStrPtr = nullptr);

public:
    bool startWork = false;
    bool mustFinish = false;
    bool ready = true;
    bool lostCSV = false;
    int threadID = 0;
    QString outStr;
    QString dbConn;
    QString filePath;
    int sleepTime = 200;
    int sleepLostTime = 2000;
    QString tableName;
    QString dbUser;
    QString dbPassword;
    QString dbAddress;
    QString dbDatabaseName;
    bool dbConnected = false;

    virtual void _doWork() = 0;
    virtual void _endWork() = 0;
    virtual void _saveForLost() = 0;
    bool dbConnect();
//    bool dbConnect(QString dbAddress, QString dbDatabase, QString dbUser, QString dbPassword);
};

#endif // DBWRITECSVTHREAD_H

