#include <windows.h>
#include <common/refcounted.hpp>

uintptr_t RefCounted::addRef()
{
    return InterlockedIncrement(&_refCount);
}

uintptr_t RefCounted::release()
{
    intptr_t res = InterlockedDecrement(&_refCount);
    if (res == 0)
    {
        delete this;
    }
    return res;
}

