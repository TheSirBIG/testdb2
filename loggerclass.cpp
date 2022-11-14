#include "loggerclass.h"
#include <iostream>

loggerClass::loggerClass(QString iniSectionName)
{
    //read ini file
    iniFile = new QSettings(QApplication::applicationName()+".ini", QSettings::IniFormat);

    iniFile->beginGroup(iniSectionName);
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
