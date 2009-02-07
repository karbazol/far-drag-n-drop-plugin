#ifndef __KARBAZOL_DRAGNDROP_2_0__ENUMFMT_H__
#define __KARBAZOL_DRAGNDROP_2_0__ENUMFMT_H__

#include "cstmdata.h"
#include "myunknown.h"

class EnumFORMATETC: public Unknown, public IEnumFORMATETC
{
private:
    FORMATETC* _fmt;
    size_t _count;
    size_t _curr;
    EnumFORMATETC(const EnumFORMATETC& r);
public:
    EnumFORMATETC(FORMATETC* fmt, size_t count, const CustomData* cstmData=0, size_t cstmCount=0);
    ~EnumFORMATETC();

    DEFINE_UNKNOWN

    HRESULT WINAPI QueryInterface(REFIID iid, void** obj);

    // Interface IEnumFORMATETC
    HRESULT WINAPI Next(ULONG celt, FORMATETC* fmt, ULONG* fetched);
    HRESULT WINAPI Skip(ULONG celt);
    HRESULT WINAPI Reset();
    HRESULT WINAPI Clone(IEnumFORMATETC** obj);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__ENUMFMT_H__
// vim: set et ts=4 ft=cpp ai :
