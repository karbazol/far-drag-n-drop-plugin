#include <shlobj.h>
#include "shutils.h"
#include "datacont.h"
/**
static void initStringInfoFromOemFileName(const wchar_t* dir, const char* fileName, MyStringW& s)
{
    size_t buffSize = lstrlen(dir) + 2 + MAX_PATH; // length(filename) and backslash and NULL
    wchar_t * buff = new wchar_t[buffSize];

    lstrcpy(buff, dir);
    wchar_t* p = buff + lstrlen(buff);
    *p++ = L'\\';

    if (!MultiByteToWideChar(CP_OEMCP, 0, fileName, -1, p, MAX_PATH))
    {
        delete [] buff;
        s = L"";
        return;
    }
    
    WIN32_FIND_DATA fd;
    HANDLE hData = FindFirstFile(buff, &fd);

    if (hData == INVALID_HANDLE_VALUE)
    {
        s = L"";
    }
    else
    {
        FindClose(hData);
        s = fd.cFileName;
    }

    delete [] buff;
    
}
/**/
DataContainer::DataContainer(const wchar_t* dir, const PluginPanelItemsW& items): 
    _dir(dir), _files(NULL), _count(items.size()),
    _custom(0), _customCount(0), _customCapacity(0)
{
    if (_count > 0)
    {
        size_t i;
        _files = new MyStringW[_count];
        for (i = 0; i < _count; i++)
        {
            /**
            if (*info.SelectedItems[i].FindData.cAlternateFileName)
            {
                initStringInfoFromOemFileName(_dir, 
                        info.SelectedItems[i].FindData.cAlternateFileName, _files[i]);
            }
            else/**/
            {
                _files[i] = items[i].FindData.cFileName;
            }
        }
    }
}

DataContainer::~DataContainer()
{
    if (_files)
        delete [] _files;
    _files = 0;
    _count = 0;

    if (_custom)
    {
        size_t i;

        for (i = 0; i < _customCount; i++)
        {
            ReleaseStgMedium(&_custom[i].mdm);
        }

        delete [] _custom;
    }
}

HGLOBAL DataContainer::createHDrop()
{
    // Calculate total length
    size_t total = sizeof(DROPFILES);
    size_t dirSize = (_dir.length()+1) * sizeof(*_dir);

    size_t i;
    for (i = 0; i < _count; i++)
    {
        total += (_files[i].length()+1) * sizeof(**_files) + dirSize;
    }

    total += 2;

    HGLOBAL res = GlobalAlloc(GHND|GMEM_SHARE, total);
    if (!res)
        return NULL;

    DROPFILES* pData = reinterpret_cast<DROPFILES*>(GlobalLock(res));
    pData->pFiles = sizeof(*pData);
    pData->fWide = TRUE;

    WCHAR* p = (WCHAR*)((char*)pData + pData->pFiles);
    for (i = 0; i < _count; i++)
    {
        lstrcpy(p, _dir);
        p += lstrlen(p);
        *p++ = L'\\';
        lstrcpy(p, _files[i]);
        p += lstrlen(p) + 1;
    }

    GlobalUnlock(res);

    return res;
}

CustomData* DataContainer::findCustom(unsigned int cf, bool append)
{
    size_t i;

    for (i = 0; i < _customCount; i++)
    {
        if (_custom[i].fmt.cfFormat == cf)
        {
            return &_custom[i];
        }
    }

    if (!append)
        return NULL;
    growCustom();

    ZeroMemory(&_custom[_customCount-1], sizeof(*_custom));

    return &_custom[_customCount-1];
}

void DataContainer::growCustom()
{
    if (_customCount == _customCapacity)
    {
        _customCapacity += 4;
        CustomData* newData = new CustomData[_customCapacity];

        size_t i;
        for (i = 0; i < _customCount; i++)
        {
            newData[i] = _custom[i];
        }

        delete [] _custom;
        _custom = newData;
    }

    _customCount++;
}

// vim: set et ts=4 ai :


