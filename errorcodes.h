#ifndef ERRORCODES_H
#define ERRORCODES_H

class errorCodes
{
public:
    static const int THREAD_END_OF_WORK = 0;
    static const int THREAD_DATABASE_OPEN = 1;
    static const int THREAD_SAVED_FOR_LOST = 2;
//    static const int THREAD_DATABASE_NOT_VALID = 1;
//    static const int THREAD_DATABASE_NOT_OPEN = 2;
    static const int THREAD_QUERY_ERROR = 3;
    static const int CLASS_NO_FREE_THREADS = 4;
    static const int NO_ERROR = 0;
};

#endif // ERRORCODES_H
