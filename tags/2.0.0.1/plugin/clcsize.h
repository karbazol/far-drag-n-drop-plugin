#ifndef __KARBAZOL_DRAGNDROP__CLCSIZE_H__
#define __KARBAZOL_DRAGNDROP__CLCSIZE_H__

#include <windows.h>

class CalcSizer
{
    __int64 _size;
    HANDLE _thread;
    void processFiles(const wchar_t* files);
    static int WINAPI run(CalcSizer* This);
public:
    CalcSizer(const wchar_t* files);
    ~CalcSizer();
    inline __int64 size(){return _size;}
    bool calculted();
};

#endif // __KARBAZOL_DRAGNDROP__CLCSIZE_H__

