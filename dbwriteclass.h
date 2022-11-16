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
// signal sig   пересылка сигнала основному обработчику от "слота" eh (в классе dbq отсутствует)
//              передает id класса, id потока, код ошибки и ,если надо, расширенную информацию в виде указателя на qstring
//
//
// класс DBWriteClass
// numOfThreads             количество созданных потоков
// csvThreadArrayCounter    счетчик для циклического запуска потоков - пока закоментировал
// csvThreadArray           массив потоков
// csvThreadReady           bool - поток свободен
// iniFile                  для чтения ini-файла, файл должен находиться на одном уровне с exe (при запуске из-под qt - на уровень выше(windows))
// db...                    для доступа к базе данных
// dbConnName               имя соединения с бд, копирует iniSectionName
// dbDatabaseName           имя базы данных
// dbTableName              имя таблицы - задается с уровня выше!!!
//
// конструктор:
// iniSectionName           название секции в ini-файле. Также это уникальное имя для объекта бд
// instID                   id класса, возможно будет полезен при отладке
// numthreads               сколько потоков запустить, по умолчанию - 5
//
// eh                       "слот" для связи с сигналом от потока. virtual - обязателен, иначе при connect будет привязываться eh от базового класса
// startWrite               запуск следующего потока. данные должны быть уже загружены в поток
//                          чтобы всегда были свободные потоки - надо прикидывать время, и запускать нужное число потоков в конструкторе
//                          возвращает false, если нет свободных потоков
// retryWrite               запуск конкретного потока
// dbConnect                соединение с бд, возвращает код и текст ошибки
// setTableName             задать имя таблицы для записи
//
//
// после конструктора класса из основной программы надо:
// задать имя таблицы, через setTableName
// вызвать dbConnect, если ошибка - повторять, пока не надоест ))) (все коды и тексты у главной программы есть)
// в процессе работы при ошибке дисконнекта (сигнал от потоков) - опять dbConnect
// в общем - потестю, распишу получше, если что...
//
// при необходимости менять имя таблицы через setTabelName
//  причины - а шут его знает, но все в одну сваливать не очень хорошо, думаю... или по размеру файла, или как-то по датам...
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

// варианты объявления DBWriteClass:
// работает глобальный динамический
//          член класса mainwindow динамический
//          член класса mainwindow статический
// не работает - глобальный статический (похоже - нужен родитель обязательно)

//для каждого типа таблицы - свой класс
//пример - dbqwe
template<class T>class DBWriteClass : public dbq
{
    int numOfThreads;
//    int csvThreadArrayCounter;
    bool* csvThreadReady;

    QSettings* iniFile;

    QString dbUser;
    QString dbPassword;
    QString dbAddress;
    QString dbConnName;
    QString dbDatabaseName;

public:
    int instanceID;
    T* csvThreadArray;
    QString dbTableName = "default";

public:
// iniSectionName - также будет connectionname для adddatabase
    DBWriteClass(QString iniSectionName, int instID, int numthreads = 5);
    virtual ~DBWriteClass();

//нужна именно virtual, чтобы переопределять в производных классах
//без virtual вызовется метод из DBWriteClass
    virtual void eh(int thrID, int errCode, QString* outStrPtr = nullptr);

    bool startWrite();
    void retryWrite(int idx);
    bool dbConnect(QSqlError::ErrorType &errType,QString &errText);
    bool execQuery(QSqlQuery &query);
    void setTableName(QString tname);
};

template<class T>
void DBWriteClass<T>::eh(int thrID, int errCode, QString* outStrPtr)
{
    csvThreadReady[thrID] = true;

    std::cout << "dbwrite we slot: " << QString::number(thrID).toStdString() << "," << QString::number(errCode).toStdString()
              << ", instanceID = " << QString::number(instanceID).toStdString()<< std::endl;
    if(outStrPtr == nullptr)
        std::cout << "null pointer" << std::endl;
    else
        std::cout << outStrPtr->toStdString() << std::endl;
    emit sig(instanceID,thrID,errCode,outStrPtr);
};

template<class T>
DBWriteClass<T>::DBWriteClass(QString iniSectionName, int instID, int numthreads)
{
    numOfThreads = numthreads;
    csvThreadArray = new T[numOfThreads];
//    csvThreadArrayCounter = 0;
    instanceID = instID;
    dbConnName = iniSectionName;
    csvThreadReady = new bool[numOfThreads];

    //prepare and start threads
    for(int i=0; i<numOfThreads; i++)
    {
        csvThreadArray[i].dbConn = dbConnName;
        csvThreadReady[i] = true;
        QObject::connect(&csvThreadArray[i], &T::finished,
                &csvThreadArray[i], &QObject::deleteLater);
        QObject::connect(&csvThreadArray[i], &T::workEnd,
                this, &DBWriteClass::eh);
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
//    iniFile->beginGroup("dbwritedata");
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
    iniFile->endGroup();
    delete iniFile;
}

template<class T>
DBWriteClass<T>::~DBWriteClass()
{
    for(int i=0; i<numOfThreads; i++)
    {
        csvThreadArray[i].mustFinish = true;
        csvThreadArray[i].wait();
    }
    delete []csvThreadArray;
    delete []csvThreadReady;
    std::cout << "class deleted" << std::endl;
    {
        QSqlDatabase db = QSqlDatabase::database(dbConnName,false);
        if(db.isOpen()) db.close();
    }
    QSqlDatabase::removeDatabase(dbConnName);
}

template<class T>
bool DBWriteClass<T>::startWrite()
{
    for(int i=0; i<numOfThreads; i++)
        if(csvThreadReady[i])
        {
            csvThreadReady[i] = false;
            csvThreadArray[i].startWork = true;
//            csvThreadArray[csvThreadArrayCounter].startWork = true;
//            csvThreadArrayCounter++;
//            if(csvThreadArrayCounter >= numOfThreads) csvThreadArrayCounter = 0;
        }
}

template<class T>
void DBWriteClass<T>::retryWrite(int idx)
{
    csvThreadArray[idx].startWork = true;
}

template<class T>
bool DBWriteClass<T>::dbConnect(QSqlError::ErrorType &errType,QString &errText)
{
    QSqlDatabase::addDatabase("QMYSQL",dbConnName);

    QSqlDatabase db = QSqlDatabase::database(dbConnName,false);
    db.setHostName(dbAddress);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    db.setDatabaseName(dbDatabaseName);

    bool retval = db.open();
    QSqlError err = db.lastError();
    errType = err.type();
    errText = err.text();
    return(retval);
}

template<class T>
void DBWriteClass<T>::setTableName(QString tname)
{
    dbTableName = tname;
    for(int i=0; i<numOfThreads; i++)
        csvThreadArray[i].tableName = dbTableName;
}


// производные классы
// м.б. - в отдельный файл выделить, а может и тут оставить, чтобы заголовков не плодить

class dbqwe : public DBWriteClass<qwe>
{
public:
    dbqwe(QString iniSectionName, int instID, int numthreads = 5):DBWriteClass(iniSectionName,instID,numthreads)
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

class logClass : public DBWriteClass<logThread>
{

public:
    logClass(QString iniSectionName):DBWriteClass(iniSectionName, 100)
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

#endif // DBWRITECLASS_H
