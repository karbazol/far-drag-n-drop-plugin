// $Id$

#include <windows.h>
#include <shlwapi.h>
#include "mystring.h"
#include "dll.h"
#include "ddlock.h"

size_t CharTraits::strlen(const CharTraits::type* p)
{
    return lstrlenA(p);
}

CharTraits::type* CharTraits::strcpy(CharTraits::type* dest, const CharTraits::type* src)
{
    return lstrcpyA(dest, src);
}

int CharTraits::strcmp(const CharTraits::type* s1, const CharTraits::type* s2)
{
    return lstrcmpA(s1, s2);
}

CharTraits::type* CharTraits::pathDelim = "\\";

size_t WideCharTraits::strlen(const WideCharTraits::type* p)
{
    return lstrlenW(p);
}

WideCharTraits::type* WideCharTraits::strcpy(WideCharTraits::type* dest, const WideCharTraits::type* src)
{
    return lstrcpyW(dest, src);
}

WideCharTraits::type* WideCharTraits::pathDelim = L"\\";

int WideCharTraits::strcmp(const WideCharTraits::type* s1, const WideCharTraits::type* s2)
{
    return lstrcmpW(s1, s2);
}

/**
 * Gets the absolute path and splits it to path and basename.
 */
bool splitAndAbsPath(const wchar_t* path, MyStringW& prefix, MyStringW& suffix)
{
    size_t size = GetFullPathName(path, 0, NULL, NULL);
    if (!size++)
        return false;

    prefix.length(size);
    wchar_t* basename;

    if (!GetFullPathName(path, size, prefix, &basename))
    {
        prefix.length(0);
        return false;
    }

    if (!basename)
    {
        // May be it is a directory?
        basename = StrRChr(prefix, NULL, L'\\');
        if (basename)
            basename++;
    }

    suffix = basename;

    if (basename)
    {
        prefix.length(basename - prefix - 1);
    }

    return true;
}

MyStringA w2a(const wchar_t* s, int cp)
{
    size_t size = WideCharToMultiByte(cp, 0, s, -1, 0, 0, NULL, NULL);

    MyStringA res;
    if (size)
    {
        res.length(size-1);

        WideCharToMultiByte(cp, 0, s, -1, static_cast<char*>(res), size, NULL, NULL);
    }

    return res;
}

MyStringW a2w(const char* s, int cp)
{
    size_t size = MultiByteToWideChar(cp, 0, s, -1, 0, 0);

    MyStringW res;
    if (size)
    {
        res.length(size-1);

        MultiByteToWideChar(cp, 0, s, -1, static_cast<wchar_t*>(res), size);
    }

    return res;
}

class MyStringCS : public CriticalSection
{
private:
    MyStringCS(): CriticalSection(){}
    ~MyStringCS(){}
    static void kill(MyStringCS* p);
public:
    static MyStringCS* instance();
};

MyStringCS* MyStringCS::instance()
{
    static MyStringCS* p = 0;
    if (!p)
    {
        p = new MyStringCS;

        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
    }

    return p;
}

void MyStringCS::kill(MyStringCS* p)
{
    delete p;
}

void myStringLock()
{
    MyStringCS::instance()->enter();
}

void myStringUnlock()
{
    MyStringCS::instance()->leave();
}

// vim: set et ts=4 ai :

