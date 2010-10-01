/**
 * @file mystring.cpp
 * Contains code for MyString classes
 *
 * $Id$
 */

#include <windows.h>
#include <shlwapi.h>
#include "mystring.h"
#include "dll.h"
#include "ddlock.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

size_t MyStringTraits<char>::strlen(const MyStringTraits<char>::type* p)
{
    return lstrlenA(p);
}

MyStringTraits<char>::type* MyStringTraits<char>::strcpy(MyStringTraits<char>::type* dest, const MyStringTraits<char>::type* src)
{
    return lstrcpyA(dest, src);
}

int MyStringTraits<char>::strcmp(const MyStringTraits<char>::type* s1, const MyStringTraits<char>::type* s2)
{
    return lstrcmpA(s1, s2);
}

const MyStringTraits<char>::type* MyStringTraits<char>::backSlash = "\\";
//const MyStringTraits<char>::type* MyStringTraits<char>::pathDelim = MyStringTraits<char>::backSlash;
/** Bug in compiler???. Optimizing compiler puts pathDelim initialization in to .CRT section
 *  when previous line used
 */
const MyStringTraits<char>::type* MyStringTraits<char>::pathDelim = "\\";
const MyStringTraits<char>::type* MyStringTraits<char>::slash = "/";
const MyStringTraits<char>::type* MyStringTraits<char>::tab = "\t";
const MyStringTraits<char>::type* MyStringTraits<char>::cr = "\r";
const MyStringTraits<char>::type* MyStringTraits<char>::lf = "\n";

size_t MyStringTraits<wchar_t>::strlen(const MyStringTraits<wchar_t>::type* p)
{
    return lstrlenW(p);
}

MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::strcpy(MyStringTraits<wchar_t>::type* dest, const MyStringTraits<wchar_t>::type* src)
{
    return lstrcpyW(dest, src);
}

int MyStringTraits<wchar_t>::strcmp(const MyStringTraits<wchar_t>::type* s1, const MyStringTraits<wchar_t>::type* s2)
{
    return lstrcmpW(s1, s2);
}

const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::backSlash = L"\\";
//const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::pathDelim = MyStringTraits<wchar_t>::backSlash;
/** Bug in compiler???. Optimizing compiler puts pathDelim initialization in to .CRT section
 *  when previous line used
 */
const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::pathDelim = L"\\";
const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::slash = L"/";
const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::tab = L"\t";
const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::cr = L"\r";
const MyStringTraits<wchar_t>::type* MyStringTraits<wchar_t>::lf = L"\n";

#endif

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

    if (!GetFullPathName(path, static_cast<DWORD>(size), prefix, &basename))
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

/**
 * Converts string from Unicode to Multibyte
 * @param[in] s Points to the source string.
 * @param[in] cp Specifies a code-page
 * @return MyStringA object representing converted string
 */
MyStringA w2a(const wchar_t* s, int cp)
{
    size_t size = WideCharToMultiByte(cp, 0, s, -1, 0, 0, NULL, NULL);

    MyStringA res;
    if (size)
    {
        res.length(size-1);

        WideCharToMultiByte(cp, 0, s, -1, static_cast<char*>(res), 
                static_cast<int>(size), NULL, NULL);
    }

    return res;
}

/**
 * Converts string from Multibyte to Unicode
 * @param[in] s Points to the source string.
 * @param[in] cp Specifies a code-page
 * @return MyStringW object representing converted string
 */
MyStringW a2w(const char* s, int cp)
{
    MyStringW res;

    if (s)
    {
        size_t size = MultiByteToWideChar(cp, 0, s, -1, 0, 0);

        if (size)
        {
            res.length(size-1);

            MultiByteToWideChar(cp, 0, s, -1, static_cast<wchar_t*>(res), 
                    static_cast<int>(size));
        }
    }

    return res;
}

/**
 * @brief Strings critical section
 *
 * Singleton. Critical section used by MyString objects
 */
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

/**
 * Acquires String critical section lock.
 */
void myStringLock()
{
    MyStringCS::instance()->lock();
}

/**
 * Releases String critical section lock.
 */
void myStringUnlock()
{
    MyStringCS::instance()->unlock();
}

// vim: set et ts=4 ai :

