#ifndef __KARBAZOL_DRAGNDROP_2_0__MYUNKNOWN_H__
#define __KARBAZOL_DRAGNDROP_2_0__MYUNKNOWN_H__

#include <windows.h>

/**
 * Default implementation of IUnknown interface.
 */
class Unknown : public IUnknown
{
private:
    ULONG _refCount;
    Unknown(const Unknown&);
public:
    Unknown(): _refCount(0){}
    virtual ~Unknown(){}

    HRESULT WINAPI QueryInterface(REFIID iid, void** obj);
    ULONG WINAPI AddRef();
    ULONG WINAPI Release();
};

#define DEFINE_UNKNOWN \
    inline ULONG WINAPI AddRef(){return Unknown::AddRef();} \
    inline ULONG WINAPI Release(){return Unknown::Release();}

#endif // __KARBAZOL_DRAGNDROP_2_0__MYUNKNOWN_H__

