#include "enumfmt.h"
#include "utils.h"

EnumFORMATETC::EnumFORMATETC(FORMATETC* fmt, size_t count, const CustomData* cstmData, size_t cstmCount)
{
    _curr = 0;
    _count = count + cstmCount;
    if (_count)
    {
        _fmt = new FORMATETC[_count];
        memmove(_fmt, fmt, count*sizeof(*_fmt));

        for (;count < _count; count++)
        {
            _fmt[count] = cstmData++->fmt;
        }
    }
    else
    {
        _fmt = NULL;
    }
    AddRef();
}

EnumFORMATETC::EnumFORMATETC(const EnumFORMATETC& r)
{
    _count = r._count;
    _curr = r._curr;

    if (_count)
    {
        _fmt = new FORMATETC[_count];
        memmove(_fmt, r._fmt, _count*sizeof(*_fmt));
    }
    else
    {
        _fmt = NULL;
    }
    AddRef();
}

EnumFORMATETC::~EnumFORMATETC()
{
    if (_fmt)
    {
        delete [] _fmt;
        _fmt = NULL;
    }
}

HRESULT EnumFORMATETC::QueryInterface(REFIID iid, void** obj)
{
    if (IsBadWritePtr(obj, sizeof(*obj)))
        return E_INVALIDARG;

    if (IsEqualGUID(iid, IID_IEnumFORMATETC))
    {
        *obj = static_cast<IEnumFORMATETC*>(this);
        AddRef();
        return S_OK;
    }

    return Unknown::QueryInterface(iid, obj);
}


HRESULT EnumFORMATETC::Next(ULONG celt, FORMATETC* fmt, ULONG* fetched)
{
    if (!celt)
        return S_FALSE;
    if (IsBadWritePtr(fmt, celt*sizeof(*fmt)))
        return E_POINTER;
    LONG res = static_cast<LONG>(min(_count - _curr, celt));
    if (res <= 0)
        return S_FALSE;
    if (fetched)
    {
        if (IsBadWritePtr(fetched, sizeof(*fetched)))
            return E_POINTER;
        *fetched = (ULONG)res;
    }

    memmove(fmt, &_fmt[_curr], res*sizeof(*_fmt));

    _curr += res;

    return S_OK;
}

HRESULT EnumFORMATETC::Skip(ULONG celt)
{
    LONG skip = static_cast<LONG>(min(_count - _curr, celt));
    if (skip <= 0)
        return S_FALSE;
    _curr += skip;
    return S_OK;
}

HRESULT EnumFORMATETC::Reset()
{
    _curr = 0;
    return S_OK;
}

HRESULT EnumFORMATETC::Clone(IEnumFORMATETC** obj)
{
    if (IsBadWritePtr(obj, sizeof(*obj)))
        return E_POINTER;

    *obj = static_cast<IEnumFORMATETC*>(new EnumFORMATETC(*this));
    return S_OK;
}

// vim: set et ts=4 ai :

