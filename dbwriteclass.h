// общая структура боевой программы
//
// в области main - tango-клиент, dim-сервер, класс <a>(один или несколько) на базе DBWriteClass, класс <l>(один или несколько) логов (тоже на DBWriteClass)
// работа
// новые данные от tango: передача по dim, запись в <a>
// при возникновении любой ошибки: передача по dim(???), запись в <l>
// если ошибка в <l>: передача по dim(???)
// вообще, практически любой чих: запись в <l>
//
// сколько выбрать потоков:
// (проверял для для logclass - там не пакетная запись)
// в этой конфигурации ожидание переподключения к бд 5 секунд (видимо, какие-то настройки драйвера по-умолчанию)
// соответственно - поток эти 5 секунд висит на dbconn, только после перейдет на 'start and no dbconn', если была запущена в него задача
// т.е. количество потоков = это примерное количество запросов в секунду * 5 и +1 для lost
// реально - делал примерно 3 раза в секунду, хватило 8 потоков ???
//
//
//
//
//
//
//
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
// csvFilePath              путь к файлам csv
//                          сделать общий ресурс на схд, подмонтировать под этим путем к машинам с mysql и этой программой!!!!!
//
//
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
// dbConnect                соединение/переподключение с бд, возвращает код и текст ошибки
//                          сбрасывает флаг соединения в потоках, т.е. они сами начинают переподключаться
// setDb...                 установка новых значений, потом надо вызвать dbConnect
// getDb...                 чтение значений
//
//
// после конструктора класса из основной программы надо:
// вызвать dbConnect, если ошибка - повторять, пока не надоест ))) (все коды и тексты у главной программы есть)
// через createTable задать имя таблицы (пусть даже существующей, там ничего не удалится)
// в процессе работы при ошибке дисконнекта (сигнал от потоков) - опять dbConnect, пока не надоест
// потоки сами соединятся с бд
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
    QString csvFilePath;

    void setTableName();
    void threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr);

protected:
    int instanceID;
    QString dbConnName;
    //нужна именно virtual, чтобы переопределять в производных классах
    //без virtual вызовется метод из DBWriteClass
    virtual bool _createTable(QString tname, QSqlError* sqlError) = 0;
    virtual void _threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr) = 0;
    int getFreeThread();
public:
    T* csvThreadArray;

public:
// iniSectionName - также будет connectionname для adddatabase
    DBWriteClass(QString iniSectionName, int instID);
    virtual ~DBWriteClass();

    QString getDbUser();
    void setDbUser(QString value);
    QString getDbPassword();
    void setDbPassword(QString value);
    QString getDbAddress();
    void setDbAddress(QString value);
    QString getDbDatabaseName();
    void setDbDatabaseName(QString value);
    QString getDbTableName();
    bool createTable(QString tname, QSqlError* sqlError);
    QString getCsvFilePath();
    void setCsvFilePath(QString value);

    bool dbConnect(QSqlError::ErrorType* errType,QString* errText);
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
    csvFilePath = iniFile->value("filepath","qqq").toString();
    if(csvFilePath == "qqq")
    {
        std::cout << "No filepath value into ini-file, created default '/mnt/common/'" << std::endl;
        csvFilePath = "/mnt/common/";
        iniFile->setValue("filepath", csvFilePath);
    }
    iniFile->endGroup();
    delete iniFile;

    csvThreadArray = new T[dbNumOfThreads];
    instanceID = instID;
    dbConnName = iniSectionName;

    //prepare and start threads
    for(int i=0; i<dbNumOfThreads; i++)
    {
        csvThreadArray[i].dbConn = dbConnName + QString::number(i);
        csvThreadArray[i].dbUser = dbUser;
        csvThreadArray[i].dbDatabaseName = dbDatabaseName;
        csvThreadArray[i].dbPassword = dbPassword;
        csvThreadArray[i].dbAddress = dbAddress;
        csvThreadArray[i].filePath = csvFilePath;
        QObject::connect(&csvThreadArray[i], &T::finished,
                &csvThreadArray[i], &QObject::deleteLater);
        QObject::connect(&csvThreadArray[i], &T::workEnd,
                this, &DBWriteClass::threadSlot);
        csvThreadArray[i].threadID = i;
        csvThreadArray[i].dbConnected = false;
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
QString DBWriteClass<T>::getDbTableName()
{
    return dbTableName;
}

template<class T>
QString DBWriteClass<T>::getDbUser()
{
    return dbUser;
}

template<class T>
void DBWriteClass<T>::setDbUser(QString value)
{
    dbUser = value;
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("user", dbUser);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].dbUser = dbUser;
}

template<class T>
QString DBWriteClass<T>::getDbPassword()
{
    return dbPassword;
}

template<class T>
void DBWriteClass<T>::setDbPassword(QString value)
{
    dbPassword = value;
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("password", dbPassword);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].dbPassword = dbPassword;
}

template<class T>
QString DBWriteClass<T>::getDbAddress()
{
    return dbAddress;
}

template<class T>
void DBWriteClass<T>::setDbAddress(QString value)
{
    dbAddress = value;
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("address", dbAddress);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].dbAddress = dbAddress;
}

template<class T>
QString DBWriteClass<T>::getDbDatabaseName()
{
    return dbDatabaseName;
}

template<class T>
void DBWriteClass<T>::setDbDatabaseName(QString value)
{
    dbDatabaseName = value;
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("database", dbDatabaseName);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].dbDatabaseName = dbDatabaseName;
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
    bool retval = true;
    QSqlDatabase::addDatabase("QMYSQL",dbConnName);

    QSqlDatabase db = QSqlDatabase::database(dbConnName,false);
    db.setHostName(dbAddress);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    db.setDatabaseName(dbDatabaseName);

    retval = db.open();
    if(!retval)
    {
        QSqlError err = db.lastError();
        *errType = err.type();
        *errText = err.text();
    }

    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].dbConnected = false;

    return retval;
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
QString DBWriteClass<T>::getCsvFilePath()
{
    return csvFilePath;
}

template<class T>
void DBWriteClass<T>::setCsvFilePath(QString value)
{
    csvFilePath = value;
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("filepath", csvFilePath);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].filePath = csvFilePath;
}

template<class T>
void DBWriteClass<T>::threadSlot(int thrID, int errCode, QString* outStrPtr)
{
    _threadSlot(thrID, errCode, outStrPtr);
    emit sig(instanceID,thrID,errCode,outStrPtr);
};

#endif // DBWRITECLASS_H
