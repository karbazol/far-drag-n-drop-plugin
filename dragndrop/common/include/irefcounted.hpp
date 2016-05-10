#ifndef __KARBAZOL_DRAGNDROP_2_0__IREFCOUNTED_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__IREFCOUNTED_HPP__

#include <stdint.h>

struct IRefCounted
{
    virtual uintptr_t addRef() = 0;
    virtual uintptr_t release() = 0;
};

#endif // __KARBAZOL_DRAGNDROP_2_0__IREFCOUNTED_HPP__
/* vim: set et ts=4 sw=4 ai : */

