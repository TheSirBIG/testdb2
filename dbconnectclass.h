#ifndef DBCONNECTCLASS_H
#define DBCONNECTCLASS_H

#include <QtSql>
//#include <QThread>
#include "dbwritecsvthread.h"
#include <iostream>

// использую новый (второй) вид QObject::connect
// в этом случае нужен только сигнал, вместо слота можно подключить любую функцию!!!
// также работает полиморфизм(???) для eh
// так что в наследовании вместо dbq поставил QObject (без него таки ругается)

/*
class dbq : public QObject
//только для возможности иметь слоты
//вроде - оба способа работают, с virtual и без
{
    Q_OBJECT
public slots:
//    virtual void eh(int thrID, int errCode)
    void eh(int thrID, int errCode)
    {
        thrID = errCode; //просто, чтобы не было предупреждений при компиляции
    };
};
*/


//работает - глобальный динамический
//           член класса mainwindow динамический
//           член класса mainwindow статический
// не работает - глобальный статический (похоже - нужен родитель обязательно)

//для каждого типа таблицы - свой класс
//пример - dbqwe
//template<class T>class DBConnectClass : public dbq
template<class T>class DBConnectClass : public QObject
{
    QSqlDatabase db;
    int numOfThreads;
    int csvThreadArrayCounter;

public:
    DBConnectClass(int instID, int numthreads = 5);
    virtual ~DBConnectClass();

//нужна именно virtual, чтобы переопределять в производных классах
//без virtual вызовется метод из DBConnectClass
    virtual void eh(int thrID, int errCode);

    void startCurrent();

    T* csvThreadArray;
    int instanceID;
};

template<class T>
void DBConnectClass<T>::eh(int thrID, int errCode)
{
    std::cout << "dbconnect we slot: " << QString::number(thrID).toStdString() << "," << QString::number(errCode).toStdString()
              << ", instanceID = " << QString::number(instanceID).toStdString()<< std::endl;
};

template<class T>
DBConnectClass<T>::DBConnectClass(int instID, int numthreads)
{
    numOfThreads = numthreads;
    csvThreadArray = new T[numOfThreads];
    csvThreadArrayCounter = 0;
    instanceID = instID;

    //prepare and start threads
    for(int i=0; i<numOfThreads; i++)
    {
        QObject::connect(&csvThreadArray[i], &T::finished,
                &csvThreadArray[i], &QObject::deleteLater);
        QObject::connect(&csvThreadArray[i], &T::workEnd,
                this, &DBConnectClass::eh);
        csvThreadArray[i].threadID = i;
        csvThreadArray[i].start();
    }
}

template<class T>
DBConnectClass<T>::~DBConnectClass()
{
    for(int i=0; i<numOfThreads; i++)
    {
        csvThreadArray[i].mustFinish = true;
        csvThreadArray[i].wait();
    }
    delete []csvThreadArray;
    std::cout << "class deleted" << std::endl;
}

template<class T>
void DBConnectClass<T>::startCurrent()
{
    csvThreadArray[csvThreadArrayCounter].startWork = true;
    csvThreadArrayCounter++;
    if(csvThreadArrayCounter >= numOfThreads) csvThreadArrayCounter = 0;
}


class dbqwe : public DBConnectClass<qwe>
{
public:
    dbqwe(int instID, int numthreads = 5):DBConnectClass(instID,numthreads)
    {
        //без этого, даже пустого, определения ругается линковщик
        //свой код, если надо
    };
    ~dbqwe()
    {
        //без этого, даже пустого, определения ругается линковщик
        //свой код, если надо
    };
    void eh(int thrID, int errCode)// override
    {
        std::cout << "dbqwe we slot: " << QString::number(thrID).toStdString() << "," << QString::number(errCode).toStdString()
                  << ", instanceID = " << QString::number(instanceID).toStdString()<< std::endl;
    };
};


#endif // DBCONNECTCLASS_H
