// $Id$

#ifndef __KARBAZOL_DRAGNDROP_2_0__FILECOPY_H__
#define __KARBAZOL_DRAGNDROP_2_0__FILECOPY_H__

#include <windows.h>
#include "mystring.h"

class FileCopier
{
public:
    struct FileCopyNotify
    {
        virtual ~FileCopyNotify(){};
        /**
         * Return true to overrwrite existing file
         */
        virtual bool onFileExists(const wchar_t* src, const wchar_t* dest) = 0;
        virtual bool onFileStep(const __int64& step) = 0;
    };
private:
    MyStringW _src;
    MyStringW _dest;
    FileCopyNotify* _notify;
    __int64 _copied;
    bool _result;
    static DWORD CALLBACK winCallBack(LARGE_INTEGER totalSize, LARGE_INTEGER transferred,
            LARGE_INTEGER streamSize, LARGE_INTEGER streamBytesTransferred,
            DWORD streamNumber, DWORD dwCallBackReason, HANDLE hSourcefile,
            HANDLE hDestinationFile, FileCopier* This);
    bool doCopy();
public:
    FileCopier(const wchar_t* src, const wchar_t* dest, FileCopyNotify* p = NULL);
    ~FileCopier(){}
    inline bool result() const {return _result;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FILECOPY_H__
