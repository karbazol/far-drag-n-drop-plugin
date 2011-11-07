/**
 * @file ddlock.h
 * The file contains declaration of various locks and synchronization objects.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__
#define __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__

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
    Lock* _lock;
    static Lock* create();
    static void destroy(Lock* lock);
public:
    CriticalSection(): _lock(0)
    {
        _lock = create();
    }
    ~CriticalSection()
    {
        destroy(_lock);
        _lock = 0;
    }

    void lock() {_lock?_lock->lock():(void)0;}
    bool tryLock() {return _lock?_lock->tryLock():false;}
    void unlock() {_lock?_lock->unlock():(void)0;}
};

/**
 * Utility class representing a scope-lock guard.
 */
template<class Lock>
class ScopeLock
{
private:
    Lock* _lock;
public:
    ScopeLock(Lock* lock): _lock(lock)
    {
        if (_lock)
        {
            _lock->lock();
        }
    }
    ~ScopeLock()
    {
        if (_lock)
        {
            _lock->unlock();
        }
    }
};

/**
 * Utility macro used by LOCKIT
 */
#define LOCKITSUFFIX(x,s) ScopeLock<Lock> l##s(&x)

/**
 * Macro to simplify ScopeLock objects usage.
 */
#define LOCKIT(x) LOCKITSUFFIX(x,__LINE__)

#endif // __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__
