#include <shlobj.h>
#include "shutils.h"
#include "datacont.h"

DataContainer::DataContainer(const wchar_t* dir, const PluginPanelItemsW& items): 
    _dir(dir), _files(NULL), _count(items.size()),
    _custom(0), _customCount(0), _customCapacity(0)
{
    if (_count > 0)
    {
        size_t i;
        _files = new MyStringW[_count];
        if (!_files)
        {
            _count = 0;
            /** @todo Raise some exception here */
            return;
        }
        for (i = 0; i < _count; i++)
        {
            _files[i] = items[i].cFileName;
        }
    }
}

DataContainer::DataContainer(const DataContainer& r):
    _dir(r._dir), _files(NULL), _count(r._count),
    _custom(NULL), _customCount(r._customCount), _customCapacity(r._customCapacity)
{
    if (_count > 0)
    {
        _files = new MyStringW[_count];
        if (_files)
        {
            size_t i;

            for (i = 0; i < _count; i++)
            {
                _files[i] = r._files[i];
            }
        }
        else
        {
            /** @todo Raise some exception here */
            _count = 0;
        }
    }

    if (_customCapacity)
    {
        _custom = new CustomData[_customCapacity];
        ZeroMemory(_custom, sizeof(*_custom)*_customCapacity);

        size_t i;

        for (i = 0; i < _customCount; i++)
        {
            _custom[i].fmt = r._custom[i].fmt;
            CopyMedium(_custom[i].mdm, r._custom[i].mdm, _custom[i].fmt.cfFormat);
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


