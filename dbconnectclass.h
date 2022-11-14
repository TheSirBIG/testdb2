#ifndef DBCONNECTCLASS_H
#define DBCONNECTCLASS_H

#include <QtSql>
//#include <QThread>
#include "dbwritecsvthread.h"
#include <iostream>
#include <QApplication>

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
    int numOfThreads;
    int csvThreadArrayCounter;
    QSqlDatabase db;
    QSettings* iniFile;
    QString dbUser;
    QString dbPassword;
    QString dbAddress;

public:
    DBConnectClass(QString iniSectionName, int instID, int numthreads = 5);
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
DBConnectClass<T>::DBConnectClass(QString iniSectionName, int instID, int numthreads)
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

    //read ini file
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
//    iniFile = new QSettings(QApplication::applicationDirPath()+"\\" + iniFileName + ".ini", QSettings::IniFormat);
//std::cout << typeid(*this).name() << std::endl;
//std::cout << typeid(this).name() << std::endl;
//std::cout << iniFile->fileName().toStdString() << std::endl;
//std::cout << QApplication::applicationDirPath().toStdString() << std::endl;
//std::cout << QApplication::applicationName().toStdString() << std::endl;
//std::cout << QApplication::applicationFilePath().toStdString() << std::endl;

    iniFile->beginGroup(iniSectionName);
//    iniFile->beginGroup("dbdata");
    dbAddress = iniFile->value("address","qqq").toString();
    if(dbAddress == "qqq")
    {
        std::cout << "No address value into ini-file, created default '127.0.0.1'" << std::endl;
        dbAddress = "127.0.0.1";
        iniFile->setValue("address", dbAddress);
    }
    dbUser = iniFile->value("user","qqq").toString();
    if(dbUser == "qqq")
    {
        std::cout << "No user value into ini-file, created default 'root'" << std::endl;
        dbUser = "root";
        iniFile->setValue("user", dbUser);
    }
    dbPassword = iniFile->value("password","qqq").toString();
    if(dbPassword == "qqq")
    {
        std::cout << "No password value into ini-file, created default '12345'" << std::endl;
        dbPassword = "12345";
        iniFile->setValue("password", dbPassword);
    }
    iniFile->endGroup();
    delete iniFile;
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
    dbqwe(QString iniSectionName, int instID, int numthreads = 5):DBConnectClass(iniSectionName,instID,numthreads)
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
