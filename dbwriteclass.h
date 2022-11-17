// общая структура боевой программы
//
// в области main - tango-клиент, dim-сервер, класс <a>(один или несколько) на базе DBWriteClass, класс <l>(один или несколько) логов (тоже на DBWriteClass)
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
#ifndef DBWRITECLASS_H
#define DBWRITECLASS_H

#include <QtSql>
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
// signal sig   пересылка сигнала основному обработчику
//              передает id класса, id потока, код ошибки и ,если надо, расширенную информацию в виде указателя на qstring
//
//
// класс DBWriteClass
// instanceID               id класса, возможно будет полезен при отладке
// dbNumOfThreads           количество созданных потоков
//      update: перенес из конструктора в ini-файл
// csvThreadArray           массив потоков
// iniFile                  для чтения ini-файла, файл должен находиться на одном уровне с exe (при запуске из-под qt - на уровень выше(windows))
// db...                    для доступа к базе данных
// dbConnName               имя соединения с бд, копирует iniSectionName
// dbDatabaseName           имя базы данных
// dbTableName              имя таблицы - задается с уровня выше!!!
//
// конструктор:
// iniSectionName           название секции в ini-файле. Также это уникальное имя для объекта бд
// instID                   id класса, возможно будет полезен при отладке
//
// setTableName             установка имени таблицы из dbTableName в потоки, вызывается из createTable
// threadSlot               "слот" для связи с сигналом от потока.
//                          используется virtual _threadSlot для дочерних классов (если не потребуется - то удалю)
//                          по факту - просто перебрасывает данные +instanceID основной программе
// getFreeThread            индекс первого свободного потока, или -1
//
// createTable              создать таблицу, передать имя в потоки
//                          используется virtual _createTable для дочерних классов
//                          должно уже быть соединение с бд
// dbConnect                соединение с бд, возвращает код и текст ошибки
//
//
// после конструктора класса из основной программы надо:
// вызвать dbConnect, если ошибка - повторять, пока не надоест(???) ))) (все коды и тексты у главной программы есть)
// задать имя таблицы, через createTable
// в процессе работы при ошибке дисконнекта (сигнал от потоков) - опять dbConnect - ???
//
// при необходимости менять имя таблицы через createTable
//  причины - а шут его знает, но все в одну сваливать не очень хорошо, думаю... или по размеру файла, или как-то по датам...
//


class dbq : public QObject
//только для возможности иметь слоты в классе с шаблоном
{
    Q_OBJECT
signals:
    void sig(int instanceID, int thrID, int errCode, QString* outStrPtr = nullptr);
};

// варианты объявления классов на базе DBWriteClass:
// работает:    глобальный динамический
//              член класса mainwindow динамический
//              член класса mainwindow статический
// не работает: глобальный статический (похоже - нужен родитель обязательно)
template<class T>class DBWriteClass : public dbq
{
    QSettings* iniFile;

    int dbNumOfThreads;
    QString dbUser;
    QString dbPassword;
    QString dbAddress;
    QString dbDatabaseName;
    QString dbTableName = "testdb2";

    void setTableName();
    void threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr);
    int getFreeThread();

protected:
    int instanceID;
    QString dbConnName;
    //нужна именно virtual, чтобы переопределять в производных классах
    //без virtual вызовется метод из DBWriteClass
    virtual bool _createTable(QString tname, QSqlError* sqlError) = 0;
    virtual void _threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr) = 0;
public:
    T* csvThreadArray;

public:
// iniSectionName - также будет connectionname для adddatabase
    DBWriteClass(QString iniSectionName, int instID);
    virtual ~DBWriteClass();

//    void retryWrite(int idx);
    bool dbConnect(QSqlError::ErrorType* errType,QString* errText);
    bool createTable(QString tname, QSqlError* sqlError);
};

template<class T>
DBWriteClass<T>::DBWriteClass(QString iniSectionName, int instID)
{
    //read ini file
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);

    iniFile->beginGroup(iniSectionName);
    dbAddress = iniFile->value("address","qqq").toString();
    if(dbAddress == "qqq")
    {
        std::cout << "No address value into ini-file, created default 'localhost'" << std::endl;
        dbAddress = "localhost";
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
    dbDatabaseName = iniFile->value("database","qqq").toString();
    if(dbDatabaseName == "qqq")
    {
        std::cout << "No database value into ini-file, created default 'testdb'" << std::endl;
        dbDatabaseName = "testdb";
        iniFile->setValue("database", dbDatabaseName);
    }
    dbNumOfThreads = iniFile->value("threads",-1).toInt();
    if(dbNumOfThreads == -1)
    {
        std::cout << "No number of threads value into ini-file, created default 6 (5 normal, 1 for 'lost')" << std::endl;
        dbNumOfThreads = 6;
        iniFile->setValue("threads", dbNumOfThreads);
    }
    iniFile->endGroup();
    delete iniFile;

    csvThreadArray = new T[dbNumOfThreads];
    instanceID = instID;
    dbConnName = iniSectionName;

    //prepare and start threads
    for(int i=0; i<dbNumOfThreads; i++)
    {
        csvThreadArray[i].dbConn = dbConnName;
        QObject::connect(&csvThreadArray[i], &T::finished,
                &csvThreadArray[i], &QObject::deleteLater);
        QObject::connect(&csvThreadArray[i], &T::workEnd,
                this, &DBWriteClass::threadSlot);
        csvThreadArray[i].threadID = i;
        csvThreadArray[i].start();
    }
    //last thread - thread for 'lost'
    csvThreadArray[dbNumOfThreads-1].ready = false;
    csvThreadArray[dbNumOfThreads-1].lostCSV = true;
}

template<class T>
DBWriteClass<T>::~DBWriteClass()
{
    for(int i=0; i<dbNumOfThreads; i++)
    {
        csvThreadArray[i].mustFinish = true;
        csvThreadArray[i].wait();
    }
    delete []csvThreadArray;
    std::cout << "class deleted" << std::endl;
    {
        QSqlDatabase db = QSqlDatabase::database(dbConnName,false);
        if(db.isOpen()) db.close();
    }
    QSqlDatabase::removeDatabase(dbConnName);
}

template<class T>
int DBWriteClass<T>::getFreeThread()
{
    int retval = -1;

    for(int i=0; i<dbNumOfThreads; i++)
        if(csvThreadArray[i].ready)
        {
            retval = i;
            break;
        }
    return retval;
}

template<class T>
bool DBWriteClass<T>::dbConnect(QSqlError::ErrorType* errType,QString* errText)
{
    QSqlDatabase::addDatabase("QMYSQL",dbConnName);

    QSqlDatabase db = QSqlDatabase::database(dbConnName,false);
    db.setHostName(dbAddress);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    db.setDatabaseName(dbDatabaseName);

    bool retval = db.open();
    QSqlError err = db.lastError();
    *errType = err.type();
    *errText = err.text();
    return(retval);
}

template<class T>
void DBWriteClass<T>::setTableName()
{
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].tableName = dbTableName;
}

template<class T>
bool DBWriteClass<T>::createTable(QString tname, QSqlError* sqlError)
{
    bool retval;

    std::cout << "dbwriteclass create table" << std::endl;
    std::cout << "call to child _createTable" << std::endl;
    retval = _createTable(tname, sqlError);
    if(retval)
    {
        std::cout << "continue create table" << std::endl;
        dbTableName = tname;
        setTableName();
    }
    return retval;
}

template<class T>
void DBWriteClass<T>::threadSlot(int thrID, int errCode, QString* outStrPtr)
{
    _threadSlot(thrID, errCode, outStrPtr);
    emit sig(instanceID,thrID,errCode,outStrPtr);
};

#endif // DBWRITECLASS_H
