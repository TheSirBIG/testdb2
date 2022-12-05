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
// реально - делал примерно 3 раза в секунду, хватило 8 потоков ??? (для logClass)
// update: для testCsvClass - 5-6 раз в секунду, судя по lost файлам - хватило 20 потоков, но вроде как впритык
//
// структура ini-файла
// файл должен находиться на одном уровне с exe (при запуске из-под qt - на уровень выше(windows))
// имя секции   имя, передаваемое в класс при создании
// address      адрес хоста mysql (по стандартному порту 3306)
// user         имя пользователя
// password     пароль пользователя
// database     имя используемой базы данных
//              база данных должна быть создана ДО запуска программы!!!
// threads      общее число запускаемых потоков (включая поток для lost)
// filepath     путь для хранения обычных и lost csv-файлов (в случае с логом - просто текстовых lost файлов)
//              делать общую папку на схд, монтировать ее к машине с mysql и к этой машине
//              в ini указывать этот путь, по нему будут писаться файлы
//              это для того, чтобы хост mysql мог использовать load infile без модификатора local!!!
//              (там и число чтения/записи будет 3, против 5 с local, да и какие-то проблемы с этим local, то работает, то ни в какую)
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef DBWRITECLASS_H
#define DBWRITECLASS_H

#include <QtSql>
#include "dbwritecsvthread.h"
#include <iostream>
#include <QApplication>

// использую новый (второй) вид функции QObject::connect
// в этом случае нужен только сигнал, вместо слота можно подключить любую функцию!!!

// класс dqb
// базовый класс для DBWriteClass, нужен только для реализации сигнала
//      (т.к. Q_OBJECT нельзя использовать вместе с шаблонами, ограничения Qt)
// функции:
// signal sig   пересылка сигнала основному обработчику
//              передает id класса, id потока, код ошибки и ,если надо, расширенную информацию в виде указателя на qstring
//
//
// класс DBWriteClass
// базовый класс для создаваемых классов, в параметре шаблона - класс потока
//      (классы потока - для каждого свои)
// переменные:
// dbNumOfThreads           количество потоков, читается из ini-файла
// db...                    для доступа к базе данных, читается из ini-файла
// dbDatabaseName           имя базы данных, читается из ini-файла
// dbTableName              имя таблицы - задается с уровня выше!!!
//                          значение по умолчанию - test_table
// csvFilePath              путь к файлам csv, читается из ini-файла
// csvSqlFilePath           путь к файлам csv для сервера mysql, читается из ini-файла
// csvThreadArray           массив потоков
// instanceID               id класса, передается в конструкторе
// dbConnName               имя соединения с бд, копирует iniSectionName, передается в конструкторе
//
// конструктор:
// iniSectionName           название секции в ini-файле. Также это уникальное имя для объекта бд
// instID                   id класса, возможно будет полезен при отладке
//
// функции:
// setTableName             установка имени таблицы из dbTableName в потоки, вызывается из createTable
// threadSlot               "слот" для связи с сигналом от потока.
//                          используется virtual _threadSlot для дочерних классов
//                          по факту - просто перебрасывает данные +instanceID основной программе
// startTimer               запуск таймера, если без параметров - то на 2 часа
// _onTimer                 virtual, соединен с сигналом таймера timeout
// getFreeThread            индекс первого свободного потока, или -1
// getDbTabelName           текущая таблица
// createTable              создать таблицу, передать имя в потоки
//                          используется virtual _createTable для дочерних классов
//                          должно уже быть соединение с бд
//                          вызывать после установки связи с бд!!!
// dbConnect                соединение/переподключение с бд, возвращает код и текст ошибки
//                          сбрасывает флаг соединения в потоках, т.е. они сами начинают переподключаться
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
{
    Q_OBJECT
    static const int twohourconst = 7200000;

protected:
    QTimer periodTimer;

signals:
    void sig(int instanceID, int thrID, int errCode, QString* outStrPtr = nullptr);

public:
    dbq()
    {
        periodTimer.setTimerType(Qt::VeryCoarseTimer);
    };
    virtual ~dbq()
    {
        periodTimer.stop();
        std::cout << "timer stopped" << std::endl;
    };
    void startTimer(int msec = twohourconst)
    {
        std::cout << "timer started with " << QString::number(msec).toStdString() << " ms" << std::endl;
        periodTimer.start(msec);
    };
};

// варианты объявления классов на базе DBWriteClass:
// работает:    глобальный динамический
//              член другого класса динамический
//              член другого класса статический
// не работает: глобальный статический (похоже - нужен родитель обязательно)
template<class T>class DBWriteClass : public dbq
{
    int dbNumOfThreads;
    QString dbUser;
    QString dbPassword;
    QString dbAddress;
    QString dbDatabaseName;
    QString dbTableName = "test_table";
    QString csvFilePath;
    QString csvSqlFilePath;

    void setTableName();
    void threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr);

public:
    T* csvThreadArray;

protected:
    int instanceID;
    QString dbConnName;
    //нужна именно virtual, чтобы переопределять в производных классах
    //без virtual вызовется метод из DBWriteClass
    virtual bool _createTable(QString tname, QSqlError* sqlError) = 0;
    virtual void _threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr) = 0;
    int getFreeThread();
    virtual void _onTimer() = 0;

public:
    DBWriteClass(QString iniSectionName, int instID);
    virtual ~DBWriteClass();

    QString getDbTableName();
    bool createTable(QString tname, QSqlError* sqlError);

/*
    QString getDbUser();
    void setDbUser(QString value);
    QString getDbPassword();
    void setDbPassword(QString value);
    QString getDbAddress();
    void setDbAddress(QString value);
    QString getDbDatabaseName();
    void setDbDatabaseName(QString value);
    QString getCsvFilePath();
    void setCsvFilePath(QString value);
*/

    bool dbConnect(QSqlError::ErrorType* errType,QString* errText);
};

template<class T>
DBWriteClass<T>::DBWriteClass(QString iniSectionName, int instID)
{
    //read ini file
    QSettings* iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(iniSectionName);
    std::cout << "Begin to read ini file" << std::endl;
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
        std::cout << "No number of threads value into ini-file, created default 10 (9 normal, 1 for 'lost')" << std::endl;
        dbNumOfThreads = 10;
        iniFile->setValue("threads", dbNumOfThreads);
    }
    if(dbNumOfThreads <= 1)
    {
        std::cout << "Must be minimum 2 threads, set to 2 (1 normal, 1 for 'lost')" << std::endl;
        dbNumOfThreads = 2;
        iniFile->setValue("threads", dbNumOfThreads);
    }
    csvFilePath = iniFile->value("filepath","qqq").toString();
    if(csvFilePath == "qqq")
    {
        std::cout << "No filepath value into ini-file, created default '/mnt/common/'" << std::endl;
        csvFilePath = "/mnt/common/";
        iniFile->setValue("filepath", csvFilePath);
    }
    csvSqlFilePath = iniFile->value("sqlfilepath","qqq").toString();
    if(csvSqlFilePath == "qqq")
    {
        std::cout << "No sqlfilepath value into ini-file, created default '/mnt/common/'" << std::endl;
        csvSqlFilePath = "/mnt/common/";
        iniFile->setValue("sqlfilepath", csvSqlFilePath);
    }
    iniFile->endGroup();
    delete iniFile;
    std::cout << "ini file was readed or created" << std::endl;

    csvThreadArray = new T[dbNumOfThreads];
    instanceID = instID;
    dbConnName = iniSectionName;

    //prepare and start threads
    for(int i=0; i<dbNumOfThreads; i++)
    {
        csvThreadArray[i].dbConn = dbConnName + QString::number(i).rightJustified(3,'0');
        csvThreadArray[i].dbUser = dbUser;
        csvThreadArray[i].dbDatabaseName = dbDatabaseName;
        csvThreadArray[i].dbPassword = dbPassword;
        csvThreadArray[i].dbAddress = dbAddress;
        csvThreadArray[i].filePath = csvFilePath;
        csvThreadArray[i].sqlFilePath = csvSqlFilePath;
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

    QObject::connect(&periodTimer, &QTimer::timeout,
            this, &DBWriteClass::_onTimer);
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
    std::cout << "Class deleted" << std::endl;
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

/*
template<class T>
QString DBWriteClass<T>::getDbUser()
{
    return dbUser;
}

template<class T>
void DBWriteClass<T>::setDbUser(QString value)
{
    dbUser = value;
    QSettings* iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
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
    QSettings* iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
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
    QSettings* iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
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
    QSettings* iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("database", dbDatabaseName);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].dbDatabaseName = dbDatabaseName;
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
    QSettings* iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);
    iniFile->beginGroup(dbConnName);
    iniFile->setValue("filepath", csvFilePath);
    iniFile->endGroup();
    delete iniFile;
    for(int i=0; i<dbNumOfThreads; i++)
        csvThreadArray[i].filePath = csvFilePath;
}
*/

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

    std::cout << "Creating table" << std::endl;
    retval = _createTable(tname, sqlError);
    if(retval)
    {
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
}

#endif // DBWRITECLASS_H
