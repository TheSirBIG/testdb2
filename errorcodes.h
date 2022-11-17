#ifndef ERRORCODES_H
#define ERRORCODES_H

class errorCodes
{
public:
    static const int THREAD_END_OF_WORK = 0;
    static const int THREAD_DATABASE_NOT_VALID = 1;
    static const int THREAD_DATABASE_NOT_OPEN = 2;
    static const int THREAD_QUERY_ERROR = 3;
    static const int CLASS_NO_FREE_THREADS = 4;
};

#endif // ERRORCODES_H
