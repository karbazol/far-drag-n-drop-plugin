#include <shlobj.h>
#include <shellapi.h>

#include <shutils.h>
#include <utils.h>
#include <dll.h>

#include "far.h"
#include "dropprcs.h"
#include "fmtprcsr.h"
#include "thrdpool.h"

DropProcessor* DropProcessor::instance()
{
    static DropProcessor* p = 0;
    if (!p)
    {
        p = new DropProcessor;
        if (p)
        {
            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
            }
        }
    }

    return p;
}

void DropProcessor::kill(DropProcessor* p)
{
    if (p)
    {
        delete p;
    }
}

HRESULT DropProcessor::processDrop(IDataObject* obj, DWORD* effect, const wchar_t* destDir)
{
    HRESULT hr = handleAsync(obj, effect, destDir);
    if (SUCCEEDED(hr))
    {
        return hr;
    }

    FormatProcessor* process = FormatProcessor::create(obj, destDir);
    if (process)
    {
        hr = (*process)(obj, effect);
        delete process;
        return hr;
    }

    return E_INVALIDARG;
}

DWORD DropProcessor::canProcess(IDataObject* _data)
{
    struct {
        CLIPFORMAT _format;
        DWORD _actions;
    } _supported[] =
    {
        {CF_HDROP, DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK},
        {CF_FILEDESCRIPTORW, DROPEFFECT_COPY | DROPEFFECT_MOVE},
        {CF_FILEDESCRIPTORA, DROPEFFECT_COPY | DROPEFFECT_MOVE},
    };

    size_t i;
    for (i = 0; i < LENGTH(_supported); i++)
    {
        FORMATETC fmt = {_supported[i]._format, NULL, DVASPECT_CONTENT, (LONG)-1, (DWORD)-1};
        if (SUCCEEDED(_data->QueryGetData(&fmt)))
            return _supported[i]._actions;
    }
    return DROPEFFECT_NONE;
}

HRESULT DropProcessor::handleAsync(IDataObject* obj, DWORD* effect, const wchar_t* destDir)
{
    HRESULT hr;

    ShPtr<IAsyncOperation> op;
    hr = obj->QueryInterface(IID_IAsyncOperation, (void**)&op);
    if (FAILED(hr)) return hr;

    BOOL async=VARIANT_FALSE;
    hr = op->GetAsyncMode(&async);
    if (FAILED(hr)) return hr;

    if (async != VARIANT_TRUE) return E_FAIL;

    hr = op->StartOperation(NULL);
    if (FAILED(hr)) return hr;

    ThreadPool* threadPool = ThreadPool::instance();

    if (!threadPool)
    {
        return E_OUTOFMEMORY;
    }

    hr = threadPool->newThread(obj, destDir, *effect);

    return hr;
}

// vim: set et ts=4 ai :

