#ifndef __KARBAZOL_DRAGNDROP_2_0__REFCOUNTED_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__REFCOUNTED_HPP__

#include <common/irefcounted.hpp>

class RefCounted: IRefCounted
{
private:
    uintptr_t _refCount;
private:
    RefCounted(const RefCounted&); // disable copy constructor
protected:
    RefCounted(): _refCount(0)
    {}
    virtual ~RefCounted(){}
public:
    uintptr_t addRef();
    uintptr_t release();
};

#endif // __KARBAZOL_DRAGNDROP_2_0__REFCOUNTED_HPP__
/* vim: set et ts=4 sw=4 ai : */

