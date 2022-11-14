#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
//#include "dbconnectclass.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    const QString username = "sirbig";
    const QString userpassword = "12345";
    const QString dbname = "testdb2";
    const QString hostname = "192.168.56.250";
//    const QString hostname = "localhost";
    const QString connname = "testconn1";

    QSqlDatabase db;

    bool connectToServer();
    void createDB();
    void closeDB();

//    DBConnectClass<DBWriteCSVThread> qqq1;
//    dbqwe qqq2;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connButton_released();

    void on_pushButton_released();

    void on_pushButton_2_released();

    void on_pushButton_3_released();

    void on_pushButton_4_released();

    void on_pushButton_5_released();

    void on_pushButton_6_released();

    void on_pushButton_7_released();

    void on_pushButton_8_released();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
