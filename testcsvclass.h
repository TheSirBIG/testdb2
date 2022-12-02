#ifndef TESTCSVCLASS_H
#define TESTCSVCLASS_H

#include "dbwriteclass.h"
#include "testcsvthread.h"

class testCsvClass : public DBWriteClass<testCsvThread>
{
    void _threadSlot(int thrID, int errCode, QString* outStrPtr = nullptr) override;
    bool _createTable(QString tname, QSqlError* sqlError) override;
public:
    testCsvClass(QString iniSectionName, int instID):DBWriteClass(iniSectionName, instID)
    {

    };
    int write(double data[100]);
};

#endif // TESTCSVCLASS_H
