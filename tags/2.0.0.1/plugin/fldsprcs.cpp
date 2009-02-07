#include <shlwapi.h>
#include "utils.h"
#include "fldsprcs.h"
#include "fldsitrt.h"
#include "shutils.h"
#include "myshptr.h"

HRESULT FileDescriptorProcessor::operator()(IDataObject* obj, DWORD*)
{
    HRESULT hr;

    FileDescriptorIterator f;

    hr = f.initialize(obj);
    if (FAILED(hr))
        return hr;

    int i = 0;
    while (f.next())
    {
        hr = copyItem(obj, f.name(), f.value(), i++);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

HRESULT FileDescriptorProcessor::handleDir(const wchar_t* name)
{
    if (dir().ensureDirectory(name))
        return S_OK;
    return HRESULT_FROM_WIN32(GetLastError());
}
    
HRESULT FileDescriptorProcessor::copyItem(IDataObject* obj, const wchar_t* name, FILEDESCRIPTOR* desc, int index)
{
    if (!name)
        return E_FAIL;

    if (desc->dwFlags & FD_ATTRIBUTES
        && desc->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return handleDir(name);
    }
    
    TRACE("Copying %S file\n", name);
    FORMATETC fmt = {CF_FILECONTENTS, 0, DVASPECT_CONTENT, (LONG)index, (DWORD)-1};
    STGMEDIUM stg = {0};

    HRESULT hr = obj->GetData(&fmt, &stg);
    if (FAILED(hr))
    {
        return hr;
    }

    switch (stg.tymed)
    {
    case TYMED_HGLOBAL:
        {
            ShPtr<IStream> stm;

            hr = CreateStreamOnHGlobal(stg.hGlobal, FALSE, &stm);
            if (SUCCEEDED(hr))
            {
                hr = handle(stm, name);
            }
        }
        break;
    case TYMED_ISTREAM:
        hr = handle(stg.pstm, name);
        break;
    case TYMED_ISTORAGE:
        hr = handle(stg.pstg, name);
        break;
    default:
        hr = DV_E_TYMED;
        break;
    }

    ReleaseStgMedium(&stg);

    return hr;
}

HRESULT FileDescriptorProcessor::handle(IStorage* stg, const wchar_t* name)
{
    HRESULT hr = S_OK;

    MyStringW s = dir().root() / name;

    ShPtr<IStorage> stgOut;
    hr = StgCreateStorageEx(s, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, STGFMT_DOCFILE,
            0, NULL, 0, IID_IStorage, (void**)&stgOut);

    if (FAILED(hr))
    {
        TRACE("StgCreateStorageEx failed\n");
        DUMPERROR(hr);
        if (hr == STG_E_FILEALREADYEXISTS)
        {
            // They give us a second chance
            // @TODO Ask user what to do with existing file
            hr = StgCreateStorageEx(s, STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
                    STGFMT_DOCFILE, 0, NULL, 0, IID_IStorage, (void**)&stgOut);
            if (FAILED(hr))
            {
                TRACE("StgCreateStorageEx failed second time\n");
                DUMPERROR(hr);

                return hr;
            }
        }
    }

    hr = stg->CopyTo(0, NULL, NULL, stgOut);

    return hr;
}

HRESULT FileDescriptorProcessor::handle(IStream* stm, const wchar_t* name)
{
    HRESULT hr = S_OK;
    MyStringW s = dir().root();

    s += L"\\";
    s += name;

    ShPtr<IStream> stmOut;

    hr = SHCreateStreamOnFileW(s, STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &stmOut);

    if (FAILED(hr))
    {
        TRACE("SHCreateStreamOnFileW failed\n");
        DUMPERROR(hr);

        return hr;
    }

    LARGE_INTEGER seekTo={0};
    stm->Seek(seekTo, STREAM_SEEK_SET, NULL);
    stmOut->Seek(seekTo, STREAM_SEEK_SET, NULL);

    STATSTG stg;
    stm->Stat(&stg, STATFLAG_NONAME);
    hr = stm->CopyTo(stmOut, stg.cbSize, NULL, NULL);

    return hr;
}

// vim: set et ts=4 ai :

