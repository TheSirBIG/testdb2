// общая структура боевой программы
//
// в области main - tango-клиент, dim-сервер, класс <a>(один или несколько) на базе DBConnectClass, класс <l>(один или несколько) логов (тоже на DBConnectClass)
// работа
// новые данные от tango: передача по dim, запись в <a>
// при возникновении любой ошибки: передача по dim(???), запись в <l>
// если ошибка в <l>: передача по dim(???)
// вообще, практически любой чих: запись в <l>
//
//
//
//
//




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
// update
// ввел наследование от dbq, чтобы посылать сигнал основному обработчику (MainWindow)

// класс dqb
// signal sig   пересылка сигнала основному обработчику от "слота" eh (в классе dbq отсутствует)
//              передает id класса, id потока, код ошибки и ,если надо, расширенную информацию в виде указателя на qstring
//
//
// класс DBConnectClass
// numOfThreads             количество созданных потоков
// csvThreadArrayCounter    счетчик для циклического запуска потоков
// iniFile                  для чтения ini-файла, файл должен находиться на одном уровне с exe (при запуске из-под qt - на уровень выше(windows))
// db...                    для доступа к базе данных
// dbConnName               имя соединения с бд, копирует iniSectionName
//
// конструктор:
// iniSectionName           название секции в ini-файле. Также это уникальное имя для объекта бд
// instID                   id класса, возможно будет полезен при отладке
// numthreads               сколько потоков запустить, по умолчанию - 5
//
// eh                       "слот" для связи с сигналом от потока. virtual - обязателен, иначе при connect будет привязываться eh от базового класса
// startWrite               запуск следующего потока. данные должны быть уже загружены в поток
//                          чтобы всегда были свободные потоки - надо прикидывать время, и запускать нужное число потоков в конструкторе
//


class dbq : public QObject
//только для возможности иметь слоты
//вроде - оба способа работают, с virtual и без
{
    Q_OBJECT
//public slots:
//    virtual void eh(int thrID, int errCode)
//    {
//        thrID = errCode; //просто, чтобы не было предупреждений при компиляции
//    };
signals:
    void sig(int instanceID, int thrID, int errCode, QString* outStrPtr = nullptr);
};

//работает - глобальный динамический
//           член класса mainwindow динамический
//           член класса mainwindow статический
// не работает - глобальный статический (похоже - нужен родитель обязательно)

//для каждого типа таблицы - свой класс
//пример - dbqwe
template<class T>class DBConnectClass : public dbq
{
    int numOfThreads;
    int csvThreadArrayCounter;
    QSettings* iniFile;

    QString dbUser;
    QString dbPassword;
    QString dbAddress;
    QString dbConnName;

public:
// iniSectionName - также будет connectionname для adddatabase
    DBConnectClass(QString iniSectionName, int instID, int numthreads = 5);
    virtual ~DBConnectClass();

//нужна именно virtual, чтобы переопределять в производных классах
//без virtual вызовется метод из DBConnectClass
    virtual void eh(int thrID, int errCode, QString* outStrPtr = nullptr);

    void startWrite();

    T* csvThreadArray;
    int instanceID;
};

template<class T>
void DBConnectClass<T>::eh(int thrID, int errCode, QString* outStrPtr)
{
    std::cout << "dbconnect we slot: " << QString::number(thrID).toStdString() << "," << QString::number(errCode).toStdString()
              << ", instanceID = " << QString::number(instanceID).toStdString()<< std::endl;
    if(outStrPtr == nullptr)
        std::cout << "null pointer" << std::endl;
    else
        std::cout << outStrPtr->toStdString() << std::endl;
    emit sig(instanceID,thrID,errCode,outStrPtr);
};

template<class T>
DBConnectClass<T>::DBConnectClass(QString iniSectionName, int instID, int numthreads)
{
    numOfThreads = numthreads;
    csvThreadArray = new T[numOfThreads];
    csvThreadArrayCounter = 0;
    instanceID = instID;
    dbConnName = iniSectionName;

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
void DBConnectClass<T>::startWrite()
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
//    ~dbqwe()
//    {
        //без этого, даже пустого, определения ругается линковщик
        //свой код, если надо
//    };
    void eh(int thrID, int errCode, QString* outStrPtr = nullptr) override
    {
        std::cout << "dbqwe we slot: " << QString::number(thrID).toStdString() << "," << QString::number(errCode).toStdString()
                  << ", instanceID = " << QString::number(instanceID).toStdString()<< std::endl;
        if(outStrPtr == nullptr)
            std::cout << "null pointer" << std::endl;
        else
            std::cout << outStrPtr->toStdString() << std::endl;
        emit sig(instanceID,thrID,errCode,outStrPtr);
    };
};

class logClass : public DBConnectClass<logThread>
{

public:
    logClass(QString iniSectionName):DBConnectClass(iniSectionName, 100)
    {
        //без этого, даже пустого, определения ругается линковщик
        //свой код, если надо
    };
    void eh(int thrID, int errCode, QString* outStrPtr = nullptr) override
    {
        if(outStrPtr == nullptr)
            std::cout << "null pointer" << std::endl;
        else
            std::cout << outStrPtr->toStdString() << std::endl;
        //do some work
        emit sig(instanceID,thrID,errCode,outStrPtr);
    }
//    QDateTime
};

#endif // DBCONNECTCLASS_H
