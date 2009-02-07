/**
 * @file ddlock.h
 * The file contains declaration of various locks and synchronization objects.
 *
 * $Id: ddlock.h 64 2008-05-11 16:57:25Z eleskine $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__
#define __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__

#include <windows.h>

/**
 * General lock interface
 */
class Lock
{
public:
    virtual ~Lock(){};
    virtual void lock() = 0;
    virtual bool tryLock();
    virtual void unlock() = 0;
};

/**
 * Represents Windows* API critical section
 */
class CriticalSection : public Lock
{
private:
    CRITICAL_SECTION _lock;
public:
    CriticalSection();
    ~CriticalSection();

    void lock();
    bool tryLock();
    void unlock();
};

/**
 * Utility class representing a scope-lock guard.
 */
class ScopeLock
{
private:
    Lock* _lock;
public:
    ScopeLock(Lock* lock): _lock(lock)
    {
        if (_lock)
            _lock->lock();
    }
    ~ScopeLock()
    {
        if (_lock)
            _lock->unlock();
    }
};

/**
 * Utility macro used by LOCKIT
 */
#define LOCKITSUFFIX(x,s) ScopeLock l##s(&x)

/**
 * Macro to simplify ScopeLock objects usage.
 */
#define LOCKIT(x) LOCKITSUFFIX(x,__LINE__)

#endif // __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__
