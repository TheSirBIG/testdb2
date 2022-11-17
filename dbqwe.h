#ifndef DBQWE_H
#define DBQWE_H

#include "dbwriteclass.h"
#include "qwethread.h"

class dbqwe : public DBWriteClass<qweThread>
{
    void _threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr) override;
    bool _createTable(QString tname, QSqlError* sqlError) override;
public:
    dbqwe(QString iniSectionName, int instID):DBWriteClass(iniSectionName,instID)
    {

    };
};

#endif // DBQWE_H
