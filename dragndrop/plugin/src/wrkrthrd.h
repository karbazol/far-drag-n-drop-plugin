/**
 * @file wrkrthrd.h
 * Contains declaration of WorkerThread class.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__
#define __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__

#include <windows.h>

#include <myshptr.h>
#include <mystring.h>

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
    DWORD _action;
    static DWORD WINAPI thread(WorkerThread* This);
    HRESULT run();
    HRESULT unpack();
public:
    WorkerThread(): _dir(), _stream(0), _obj(), _action(DROPEFFECT_NONE) {}
    ~WorkerThread(){}
    HRESULT execute(IDataObject* obj, const wchar_t* destDir, DWORD action);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__WRKRTHRD_H__

