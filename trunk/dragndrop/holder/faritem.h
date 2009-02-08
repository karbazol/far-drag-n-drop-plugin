/**
 * @file faritem.h
 * Contains declaration of FarItem class.
 */
#ifndef __KARABAZOL_DRAGNDROP_2_0__FARITEM_H__
#define __KARABAZOL_DRAGNDROP_2_0__FARITEM_H__

#include <windows.h>
#include "growarry.h"

/**
 * Array of window handles
 */
typedef GrowOnlyArray<HWND> HwndsArray;

/**
 * Represents one Far item in the holder process.
 */
class FarItem
{
private:
    HWND _hwnd;
    HwndsArray _dnds;
public:
    FarItem():_hwnd(0), _dnds(){}
    FarItem(const HWND& h): _hwnd(h), _dnds(){}
    FarItem(const FarItem& r): _hwnd(r._hwnd), _dnds(r._dnds){}
    ~FarItem(){}
    inline FarItem& operator=(const FarItem& r)
    {
        _hwnd = r._hwnd;
        _dnds = r._dnds;
        return *this;
    }
    inline HWND hwnd() const {return _hwnd;}
    inline void hwnd(HWND value) {_hwnd = value;}
    inline const HwndsArray& dnds() const {return _dnds;}
    inline HwndsArray& dnds() {return _dnds;}
};

#endif // __KARABAZOL_DRAGNDROP_2_0__FARITEM_H__

