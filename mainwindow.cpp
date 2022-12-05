#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
//#include <Windows.h>

#include "logclass.h"
#include "testcsvclass.h"

//  !!!!!!!!!!!!!!!!!! query.exec("set global local_infile=1");     !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! на всякий случай вставлять после открытия db !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! это чтобы сработала команда load data local  !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! где-то в исходниках в dim есть пример        !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! но можно и без local попробовать             !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!!                                              !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! файл csv буду делать                         !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! на общем ресурсе типа nfs/iscsi,             !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! который подмонтирую в машину с mysql         !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! в этом случае число записей/чтения           !!!!!!!!!!!!!!!!!!!!!!!!!
//  !!!!!!!!!!!!!!!!!! с схд будет 3, а с локальным будет 5         !!!!!!!!!!!!!!!!!!!!!!!!!

const int numOfCollumn = 100;
//const int numOfIteration = 10000;
//DBWriteClass* qqq;
//DBWriteClass<DBWriteCSVThread>* qqq1;
//dbqwe* qwe;
logClass* lc;
testCsvClass* tcc;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)//,qqq1(1,4),qqq2(2,4)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->checkBoxOk->setCheckState(Qt::Unchecked);
    ui->textEdit->clear();
}

MainWindow::~MainWindow()
{
    closeDB();
//    delete qqq1;
//    delete qqq2;
//    delete qwe;
    delete lc;
    delete tcc;
    delete ui;
}

void MainWindow::closeDB()
{
//    QSqlDatabase::database(connname).close();
//    db.close();
    QSqlDatabase::removeDatabase(connname);
}

void MainWindow::sigFrom(int instanceID, int thrID, int errCode,  QString* outStrPtr)
{
    std::cout << "signal from process: " << QString::number(thrID).toStdString() << "," << QString::number(errCode).toStdString()
              << ", instanceID = " << QString::number(instanceID).toStdString()<< std::endl;
    if(outStrPtr == nullptr)
        std::cout << "null pointer at main" << std::endl;
    else
        std::cout << outStrPtr->toStdString() << std::endl;
}

void MainWindow::createDB()
{
    bool res;

    QSqlDatabase db = QSqlDatabase::database(connname);

    ui->textEdit->insertPlainText("connect to database\n");
    res = connectToServer();
    if(!res)
    {
        QString strcode;
        int code;

        ui->textEdit->insertPlainText("some error, try to resolve\n");
        strcode = db.lastError().nativeErrorCode();
        code = strcode.toInt();
        ui->textEdit->insertPlainText(db.lastError().text()+", "+strcode+"\n");
        if(code == 1049) //no database exists
        {
            ui->textEdit->insertPlainText("code: no DB, try to create\n");
            QProcess proc;
            QString cmdline;
            cmdline = "mysql -u" + username + " -p" + userpassword;
            cmdline += " -h" +hostname + " -e \"CREATE DATABASE testdb2 DEFAULT CHARACTER SET cp1251 COLLATE cp1251_general_ci;\"";
            ui->textEdit->insertPlainText(cmdline+"\n");
            proc.start(cmdline);
            proc.waitForFinished(-1);
            if(!connectToServer())
            {
                strcode = db.lastError().nativeErrorCode();
                code = strcode.toInt();
                if(code == 1049) //no database exists
                {
                    ui->textEdit->insertPlainText("still problem created database\n");
                }
            }
            else
            {
                ui->textEdit->insertPlainText("created and connected succesfull\n");
                res = true;
            }
        }
    }
    if(res)
    {
//        db.open();
        ui->textEdit->insertPlainText("check table\n");
        ui->textEdit->repaint();
        QSqlQuery query(db);
        query.exec("show table status where Name=\"test_table2\"");
        ui->textEdit->insertPlainText(query.lastError().text()+"\n");
        if(query.first())
        {
            ui->textEdit->insertPlainText("table exists\n");
        }
        else
        {
            ui->textEdit->insertPlainText("table not exists, creating\n");
            query.exec("create table test_table2 (id bigint unsigned auto_increment primary key) engine=innodb");
            QString col1,col2;
            col1="id";
            for(int i=0; i<numOfCollumn; i++)
            {
                if(i<10) col2 = "col00"; else if(i<100) col2 = "col0"; else col2 = "col";
                col2 += QString::number(i);
                query.exec("alter table test_table2 add column " + col2 + " double null after " + col1);
                col1 = col2;
                ui->textEdit->insertPlainText("column added\n");
                ui->textEdit->repaint();
            }
            ui->textEdit->insertPlainText("table created with " + QString::number(numOfCollumn) + " columns\n");
        }
//        query.exec("set global local_infile=1");
    }
}

bool MainWindow::connectToServer()
{
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QMYSQL",connname);

    db.setHostName(hostname);
    db.setDatabaseName(dbname);
    db.setUserName(username);
    db.setPassword(userpassword);

    return db.open();
}

void MainWindow::on_connButton_released()
{
//    const QSqlResult *res;

    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QMYSQL","conn_name1");

    db.setHostName("sirbig");
    db.setDatabaseName("testdb");
    db.setUserName("testdb");
    db.setPassword("12345");

    bool ok = db.open();
    if(ok) ui->checkBoxOk->setCheckState(Qt::Checked);    
//QSqlDatabase::lastError();
    ui->textEdit->insertPlainText(db.lastError().text());

}

void MainWindow::on_pushButton_released()
{
//    db.close();
//    db.removeDatabase("conn_name1");
//    db.removeDatabase(qs);
    QSqlDatabase::removeDatabase("conn_name1");
}

void MainWindow::on_pushButton_2_released()
{
    QString sss;

    //    QSqlQuery query;
    QSqlDatabase db = QSqlDatabase::database(connname);
        QSqlQuery query(db);

        query.exec("select * from test_table1");
        ui->textEdit->insertPlainText(db.lastError().text());

    //    query.exec("SELECT name, salary FROM employee WHERE salary > 50000");
    //  посмотреть, как делать loadtable, или как его там... !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //QSqlTableModel
        if(query.isSelect())
            while(query.next())
            {
                sss = query.value(0).toString();
                sss += ",";
                sss += query.value(1).toString();
                sss += ",";
                sss += query.value(2).toString();
                sss += "\n";
                ui->textEdit->insertPlainText(sss);
            }
}

void MainWindow::on_pushButton_3_released()
{
//    db.removeDatabase("conn_name1");
}

void MainWindow::on_pushButton_4_released()
{
    createDB();
}

void MainWindow::on_pushButton_5_released()
{
    QFile csv1("d:/virtual.machines/common/csv1.csv");
    QFile csv2("d:/csv2.csv");
    QTextStream stream(&csv1);
//    double val;
    QTime time;
    int elapsed;
    QString ss1;
    std::string ss2;
    QSqlQuery query(QSqlDatabase::database(connname));

/*
    time.start();
    csv1.open(QFile::WriteOnly);
    for(int j=0; j<numOfIteration; j++)
    {
        ss1 = "0;";
        for(int i=0; i<numOfCollumn; i++)
        {
            val = (double)(i+j)/300;
            ss1 += QString::number(val);
            if(i==numOfCollumn-1) ss1+="\n"; else ss1+=";";
        }
        stream << ss1;
    }
    stream.flush();
    csv1.close();
    elapsed = time.elapsed();
    ui->textEdit->insertPlainText("create file - "+QString::number(elapsed)+"\n");
    time.restart();
*/
time.start();
//query.exec("set global local_infile=1");
//    query.exec("LOAD DATA LOCAL INFILE 'd:/csv1.csv' INTO TABLE test_table2 FIELDS TERMINATED BY ';' LINES TERMINATED BY '\n' SET id=DEFAULT");
    query.exec("LOAD DATA INFILE '/media/sf_Common/csv1.csv' INTO TABLE test_table2 FIELDS TERMINATED BY ';' LINES TERMINATED BY '\n' SET id=DEFAULT");
//    query.exec("LOAD DATA INFILE '/0/csv1.csv' INTO TABLE test_table2 FIELDS TERMINATED BY ';' LINES TERMINATED BY '\n' SET id=DEFAULT");
    ui->textEdit->insertPlainText(query.lastError().text()+"\n");
    elapsed = time.elapsed();
    ui->textEdit->insertPlainText("write to db - "+QString::number(elapsed)+"\n");
/*
    time.restart();
    csv2.open(QFile::WriteOnly);
    for(int j=0; j<numOfIteration; j++)
    {
        ss2 = "0;";
        for(int i=0; i<numOfCollumn; i++)
        {
            val = (double)(i+j)/300;
            ss2 += std::to_string(val);
            if(i==numOfCollumn-1) ss2+="\n"; else ss2+=";";
        }
        csv2.write(ss2.c_str());
    }
    csv2.close();
    elapsed = time.elapsed();
    ui->textEdit->insertPlainText(QString::number(elapsed)+"\n");
*/
}

void MainWindow::on_pushButton_6_released()
{
//    DBWriteClass* qq1;
//    DBWriteClass* qq2;
//    qq1 = new DBWriteClass();
//    qq2 = new DBWriteClass(10);
//    std::cout << qq1->numOfThreads << std::endl;
//    std::cout << qq2->numOfThreads << std::endl;
//    delete qq1;
//    delete qq2;

//    qqq1 = new DBWriteClass<DBWriteCSVThread>("DBWriteClass",1,4);
//    qwe = new dbqwe("dbqwe",2);
    lc = new logClass("lc", 100);
    tcc = new testCsvClass("testcsv", 1);

//    QObject::connect(qqq1, &dbq::sig,

//    QObject::connect(qwe, &dbq::sig,
//            this, &MainWindow::sigFrom);
//    QObject::connect(qqq2, &dbq::sig,
    QObject::connect(lc, &dbq::sig,
            this, &MainWindow::sigFrom);
    QObject::connect(tcc, &dbq::sig,
            this, &MainWindow::sigFrom);

//    qqq2->createTable("test_table1");
//    qqq2->dbConnect(etype,etext);
//    qwe->createTable("test_table1");
//    qwe->dbConnect(etype,etext);
//std::cout << etype << "," << etext.toStdString() << std::endl;
    QSqlError::ErrorType etype;
    QString etext;
    bool b = false;
    while(!b)
    {
        b = lc->dbConnect(&etype,&etext);
        if(!b)
        {
            std::cout << "error in 'lc' connecting to db, error text: " << etext.toStdString() << std::endl;
        }
        this->thread()->sleep(1);
    }
    QSqlError err;
    b = lc->createTable("logtable_00001",&err);
    if(!b)
    {
        std::cout << "error in main 'lc' creating table, error text: " << err.text().toStdString() << std::endl;
    }

    b = false;
    while(!b)
    {
        b = tcc->dbConnect(&etype,&etext);
        if(!b)
        {
            std::cout << "error in 'tcc' connecting to db, error text: " << etext.toStdString() << std::endl;
        }
        this->thread()->sleep(1);
    }
    b = tcc->createTable("testcsv_01",&err);
    if(!b)
    {
        std::cout << "error in main 'tcc' creating table, error text: " << err.text().toStdString() << std::endl;
    }

    lc->startTimer(2000);
    tcc->startTimer(3000);
}

void MainWindow::on_pushButton_9_released()
{
    static int q = 0;

    lc->write("test-" + QString::number(q++));
}

void MainWindow::on_pushButton_10_released()
{
    static double q = 0.1;
    double data[100];

    for(int i=0; i<100; i++) data[i] = q;
    tcc->write(data);
    q += 0.1;
}
