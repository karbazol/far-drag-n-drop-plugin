// $Id$

#include "ddlock.h"

bool Lock::tryLock()
{
    lock();
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

void CriticalSection::lock()
{
    EnterCriticalSection(&_lock);
}

bool CriticalSection::tryLock()
{
    return !!TryEnterCriticalSection(&_lock);
}

void CriticalSection::unlock()
{
    LeaveCriticalSection(&_lock);
}

// vim: set et ts=4 ai :

