/**
 * @file myunknown.cpp
 * The file contains implementation for Unknown class.
 *
 * $Id: myunknown.cpp 77 2011-10-22 20:25:46Z Karbazol $
 */

#include <common/myunknown.h>
#include <common/utils.h>

/**
 * Returns pointers to supported interfaces. 
 */
HRESULT Unknown::QueryInterface(REFIID iid, void** obj)
{
    TRACEIID(&iid);
    if (IsBadWritePtr(obj, sizeof(*obj)))
        return E_INVALIDARG;
    if (IsEqualGUID(iid, IID_IUnknown))
    {
        *obj = this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

#ifdef _DEBUG

class UnknownRefCounter
{
private:
    LONG _instanceCount;
    LONG _maxCount;
    UnknownRefCounter():_instanceCount(0), _maxCount(0){}
public:
    ~UnknownRefCounter()
    {
        TRACE("Exiting with %d instances from %d totally reffered\n", 
            _instanceCount-1, _maxCount-1);
    }
    static UnknownRefCounter* instance();
    void AddRef()
    {
        InterlockedIncrement(&_instanceCount);
        if (_maxCount < _instanceCount)
        {
            _maxCount = _instanceCount;
        }
    }
    void DecRef()
    {
        InterlockedDecrement(&_instanceCount);
    }
};

UnknownRefCounter* UnknownRefCounter::instance()
{
    static UnknownRefCounter rc;

    return &rc;
}
#endif

/**
 * Increments internal reference counter.
 */
ULONG Unknown::AddRef()
{
#ifdef _DEBUG
    UnknownRefCounter::instance()->AddRef();
#endif
    
    return InterlockedIncrement(reinterpret_cast<LONG*>(&_refCount));
}

/**
 * Decrements internal reference counter.
 */
ULONG Unknown::Release()
{
#ifdef _DEBUG
    UnknownRefCounter::instance()->DecRef();
#endif

    ULONG res;
    if (!(res = InterlockedDecrement(reinterpret_cast<LONG*>(&_refCount))))
    {
        delete this;
    }

    return res;
}
// vim: set et ts=4 ai :

