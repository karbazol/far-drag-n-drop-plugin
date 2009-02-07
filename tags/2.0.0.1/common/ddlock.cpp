// $Id$

#include "ddlock.h"

bool Lock::tryEnter()
{
    enter();
    return true;
}

CriticalSection::CriticalSection()
{
    InitializeCriticalSection(&_lock);
}

CriticalSection::~CriticalSection()
{
    DeleteCriticalSection(&_lock);
}

void CriticalSection::enter()
{
    EnterCriticalSection(&_lock);
}

bool CriticalSection::tryEnter()
{
    return !!TryEnterCriticalSection(&_lock);
}

void CriticalSection::leave()
{
    LeaveCriticalSection(&_lock);
}

lockIt::lockIt(Lock* lock): _lock(lock)
{
    if (_lock)
        _lock->enter();
}

lockIt::~lockIt()
{
    if (_lock)
        _lock->leave();
}

// vim: set et ts=4 ai :

