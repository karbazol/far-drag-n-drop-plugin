// $Id: dircrtr.cpp 78 2008-11-01 16:57:30Z eleskine $
#include <windows.h>
#include "dircrtr.h"

DirectoryCreator::DirectoryCreator(const wchar_t* dir): _root(dir)
{
    _root += L"\\";

    for (wchar_t* p = _root; *p; p++)
    {
        if (*p == L'/') *p = L'\\';
    }
}

bool DirectoryCreator::ensureDirectory(const wchar_t* subdir, unsigned int attr) const
{
    if (checkDirectory(subdir))
        return true;
    return createDirectory(subdir, attr);
}

bool DirectoryCreator::checkDirectory(const wchar_t* subdir) const
{
    MyStringW theDir = _root / subdir;

    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(theDir, &fd);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    FindClose(h);

    if (FILE_ATTRIBUTE_DIRECTORY != (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        SetLastError(ERROR_ALREADY_EXISTS);
        return false;
    }
    return true;
}

bool DirectoryCreator::createDirectory(const wchar_t* subdir, unsigned int attr) const
{
    MyStringW theDir = _root / subdir;

    bool res = CreateDirectory(theDir, NULL)?true:false;

    if (res)
    {
#if defined(_DEBUG)
        if (!
#endif
                SetFileAttributes(theDir, attr)
#if defined(_DEBUG)
           )
        {
            LASTERROR();
        }
#else
        ;
#endif
        return res;
    }

    return createDirectoryRecursive(subdir);
}

bool DirectoryCreator::createDirectoryRecursive(const wchar_t* /*subdir*/) const
{
    /** @todo Implement recursive creation of directories */
    SetLastError(ERROR_PATH_NOT_FOUND);
    return false;
}

// vim: set et ts=4 ai :

