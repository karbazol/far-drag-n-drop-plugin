#include "fldsitrt.h"
#include "shutils.h"

FileDescriptorIterator::FileDescriptorIterator(): _count(0), _current(-1), _pcurrent(0), _step(0)
{
    memset(&_medium, 0, sizeof(_medium));
    _name = &FileDescriptorIterator::nameFromW;
}

FileDescriptorIterator::~FileDescriptorIterator()
{
    if (_pcurrent)
        ReleaseStgMedium(&_medium);
}

HRESULT FileDescriptorIterator::initialize(IDataObject* obj)
{
    FORMATETC fmt =
    {
        CF_FILEDESCRIPTORW,
        NULL,
        DVASPECT_CONTENT,
        (LONG)-1,
        (DWORD)-1
    };

    HRESULT hr = obj->GetData(&fmt, &_medium);

    _step = sizeof(FILEDESCRIPTORW);

    if (FAILED(hr))
    {
        _name = &FileDescriptorIterator::nameFromA;
        fmt.cfFormat = CF_FILEDESCRIPTORA;
        _step = sizeof(FILEDESCRIPTORA);

        hr = obj->GetData(&fmt, &_medium);
    }

    if (SUCCEEDED(hr))
    {
        _pcurrent = (char*)GlobalLock(_medium.hGlobal);
        _count = ((FILEGROUPDESCRIPTOR*)_pcurrent)->cItems;
        _pcurrent += sizeof(((FILEGROUPDESCRIPTOR*)_pcurrent)->cItems);
        _pcurrent -= _step;
    }

    return hr;
}

MyStringW FileDescriptorIterator::nameFromW()
{
    return MyStringW(((FILEDESCRIPTORW*)_pcurrent)->cFileName);
}

MyStringW FileDescriptorIterator::nameFromA()
{
    return a2w(((FILEDESCRIPTORA*)_pcurrent)->cFileName, CP_ACP);
}

bool FileDescriptorIterator::next()
{
    if (++_current >= _count)
        return false;

    _pcurrent += _step;

    return true;
}

// vim: set et ts=4 ai :

