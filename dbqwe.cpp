#include "dbqwe.h"


void dbqwe::_threadSlot(int thrID, int errCode, QString *outStrPtr)
{
    std::cout << "dbqwe slot from thread: " << QString::number(thrID).toStdString() << ", code: " <<
                 QString::number(errCode).toStdString() << std::endl;
    if(outStrPtr == nullptr)
        std::cout << "dbqwe outstr null pointer" << std::endl;
    else
        std::cout << outStrPtr->toStdString() << std::endl;
}

bool dbqwe::_createTable(QString tname, QSqlError* sqlError)
{
    bool retval;

    std::cout << "dbqwe _createtable, name = " << tname.toStdString() << std::endl;
    return retval;
}
