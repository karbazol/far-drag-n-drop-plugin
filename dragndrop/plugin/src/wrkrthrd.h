/**
 * @file wrkrthrd.h
 * Contains declaration of WorkerThread class.
 *
 * $Id: wrkrthrd.h 77 2011-10-22 20:25:46Z Karbazol $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__
#define __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__

#include <windows.h>
#include "mystring.h"
#include "myshptr.h"

/**
 * @brief Worker thread object.
 *
 * Implements a worker thread object used by ThreadPool to perform various 
 * asynchronous actions.
 */
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

