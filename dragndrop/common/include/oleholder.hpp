#ifndef __KARBAZOL_DRAGNDROP_2_0__OLEHOLDER_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__OLEHOLDER_HPP__

#include <windows.h>

class OleHolder
{
private:
    HRESULT _result;
public:
    OleHolder()
    {
        _result = OleInitialize(NULL);
    }
    ~OleHolder()
    {
        OleUninitialize();
    }
    HRESULT result() const {return _result;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__OLEHOLDER_HPP__
// vim: set et ts=4 sw=4 ai :

