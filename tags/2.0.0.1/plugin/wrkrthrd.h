#ifndef __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__
#define __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__

#include <windows.h>
#include "mystring.h"
#include "myshptr.h"

class WorkerThread
{
private:
    MyStringW _dir;
    IStream* _stream;
    ShPtr<IDataObject> _obj;
    static DWORD WINAPI thread(WorkerThread* This);
    HRESULT run();
    HRESULT unpack();
public:
    WorkerThread(): _dir(), _stream(0), _obj(){}
    ~WorkerThread(){}
    HRESULT execute(IDataObject* obj, const wchar_t* destDir);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__

