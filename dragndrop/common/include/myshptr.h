#ifndef __KARBAZOL_DRAGNDROP_2_0__MYSHPTR_H__
#define __KARBAZOL_DRAGNDROP_2_0__MYSHPTR_H__

#include <ShlObj.h>

/**
 * Template class used to manage interfaces derrived from IUnknown.
 */
template <class I>
class ShPtr
{
private:
    I* _p;
    void release()
    {
        if (_p)
            _p->Release();
        _p = 0;
    }
    void addRef()
    {
        if (_p)
            _p->AddRef();
    }
public:
    ShPtr(): _p(0){}
    ShPtr(I* p): _p(p){}
    ShPtr(const ShPtr& r): _p(r._p)
    {
        addRef();
    }
    const ShPtr& operator=(const ShPtr& r)
    {
        release();
        _p = r._p;
        addRef();
        return *this;
    }
    const ShPtr& operator=(I* p)
    {
        release();
        _p = p;
        addRef();
        return *this;
    }
    ~ShPtr()
    {
        release();
    }
    I* operator->(){return _p;}
    const I* operator->()const{return _p;}
    I** operator&(){return &_p;}
    operator I*(){return _p;}
};

#ifdef __IDataObjectAsyncCapability_INTERFACE_DEFINED__
// are you having fun, Microsoft?
#define IAsyncOperation IDataObjectAsyncCapability
#define IID_IAsyncOperation IID_IDataObjectAsyncCapability
#endif

#endif // __KARBAZOL_DRAGNDROP_2_0__MYSHPTR_H__
