// $Id$
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

bool DirectoryCreator::ensureDirectory(const wchar_t* subdir) const
{
    if (checkDirectory(subdir))
        return true;
    return createDirectory(subdir);
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

bool DirectoryCreator::createDirectory(const wchar_t* subdir) const
{
    MyStringW theDir = _root / subdir;

    bool res = CreateDirectory(theDir, NULL)?true:false;

    if (res)
    {
        return res;
    }

    return createDirectoryRecursive(subdir);
}

bool DirectoryCreator::createDirectoryRecursive(const wchar_t* /*subdir*/) const
{
    SetLastError(ERROR_PATH_NOT_FOUND);
    return false;
}

// vim: set et ts=4 ai :

