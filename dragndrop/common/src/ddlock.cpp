// $Id: ddlock.cpp 77 2011-10-22 20:25:46Z Karbazol $

#include <windows.h>
#include "ddlock.h"

bool Lock::tryLock()
{
    lock();
    return true;
}

class CriticalSectionImpl: public Lock
{
private:
    CRITICAL_SECTION _lock;
public:
    CriticalSectionImpl()
    {
        InitializeCriticalSection(&_lock);
    }

    ~CriticalSectionImpl()
    {
        DeleteCriticalSection(&_lock);
    }

    void lock()
    {
        EnterCriticalSection(&_lock);
    }

    bool tryLock()
    {
        return !!TryEnterCriticalSection(&_lock);
    }

    void unlock()
    {
        LeaveCriticalSection(&_lock);
    }
};

Lock* CriticalSection::create()
{
    return new CriticalSectionImpl();
}

void CriticalSection::destroy(Lock* lock)
{
    if (lock)
    {
        delete lock;
    }
}

// vim: set et ts=4 ai :

