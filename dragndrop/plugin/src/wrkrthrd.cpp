#include <oleholder.hpp>

#include "wrkrthrd.h"
#include "fmtprcsr.h"

HRESULT WorkerThread::execute(IDataObject* obj, const wchar_t* destDir)
{
    _dir = destDir;

    HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, obj, &_stream);

    if (FAILED(hr))
    {
        delete this;
        return hr;
    }

    DWORD id;
    HANDLE hThread = CreateThread(NULL, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(&thread), this, 0, &id);

    if (!hThread)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        delete this;
    }
    else
    {
        CloseHandle(hThread);
        hr = S_OK;
    }

    return hr;
}

DWORD WorkerThread::thread(WorkerThread* This)
{
    OleHolder oleHolder;

    if (SUCCEEDED(oleHolder.result()))
    {
        This->run();
    }
    delete This;
    return 0;
}

HRESULT WorkerThread::run()
{
    HRESULT hr = unpack();
    if (FAILED(hr))
        return hr;

    DWORD effect = DROPEFFECT_NONE;
    FormatProcessor* process = FormatProcessor::create(_obj, _dir);
    if (process)
    {
        hr = (*process)(_obj, &effect);
        delete process;
    }
    else
    {
        hr = E_FAIL;
    }

    ShPtr<IAsyncOperation> op;
    if (SUCCEEDED(_obj->QueryInterface(IID_IAsyncOperation, (void**)&op)))
    {
        op->EndOperation(hr, NULL, effect);
    }

    return hr;
}

HRESULT WorkerThread::unpack()
{
    HRESULT hr = CoGetInterfaceAndReleaseStream(_stream, IID_IDataObject, (void**)&_obj);
    _stream = NULL;

    return hr;
}

// vim: set et ts=4 ai :

