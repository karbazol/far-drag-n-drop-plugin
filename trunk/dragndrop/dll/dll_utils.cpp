/**
 * @file dll_utils.cpp
 * The file contains definations of utility classes and routines used by DLLs.
 *
 * $Id$
 */

#include <windows.h>
#include "dll_utils.h"

MyStringW getErrorDesc(unsigned int e)
{
    MyStringW res;

    wchar_t* msg = 0;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ARGUMENT_ARRAY,
            NULL, e, 0, (LPWSTR)&msg, 0, NULL);

    if (msg)
    {
        size_t i = lstrlen(msg);
        while(i > 0)
        {
            --i;
            if (msg[i] != L'\n' && msg[i] != L'\r')
            {
                break;
            }
            msg[i] = 0;
        }

        res = msg;

        LocalFree(msg);
    }

    return res;
}

MyStringW getLastErrorDesc()
{
    DWORD e = GetLastError();

    MyStringW res = getErrorDesc(e);

    SetLastError(e);

    return res;
}

// vim: set et ts=4 sw=4 ai :
