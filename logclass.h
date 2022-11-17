#ifndef LOGCLASS_H
#define LOGCLASS_H

#include "dbwriteclass.h"
#include "logthread.h"

class logClass : public DBWriteClass<logThread>
{
    void _threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr) override;
    bool _createTable(QString tname, QSqlError* sqlError) override;
public:
    logClass(QString iniSectionName):DBWriteClass(iniSectionName, 100)
    {

    };
};

#endif // LOGCLASS_H
