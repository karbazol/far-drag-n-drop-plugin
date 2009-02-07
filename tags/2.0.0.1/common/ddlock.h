// $Id$

#ifndef __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__
#define __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__

#include <windows.h>

class Lock
{
public:
    virtual ~Lock(){};
    virtual void enter() = 0;
    virtual bool tryEnter();
    virtual void leave() = 0;
};

class CriticalSection : public Lock
{
private:
    CRITICAL_SECTION _lock;
public:
    CriticalSection();
    ~CriticalSection();

    void enter();
    bool tryEnter();
    void leave();
};

class lockIt
{
private:
    Lock* _lock;
public:
    lockIt(Lock* lock);
    ~lockIt();
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DDLOCK_H__
