#include <shlwapi.h>

#include <common/myshptr.h>
#include <common/shutils.h>
#include <common/utils.h>

#include "fldsprcs.h"
#include "fldsitrt.h"

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
        hr = copyItem(obj, f, i++);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

HRESULT FileDescriptorProcessor::handleDir(FileDescriptorIterator& file)
{
    FILEDESCRIPTOR* value = file.value();
    if (!value)
    {
        return E_POINTER;
    }

    if (dir().ensureDirectory(file.name(), value->dwFileAttributes))
    {
        return S_OK;
    }

    return HRESULT_FROM_WIN32(GetLastError());
}

HRESULT FileDescriptorProcessor::copyItem(IDataObject* obj, FileDescriptorIterator& file, int index)
{
    if (!file.name())
    {
        return E_FAIL;
    }

    if (!obj)
    {
        return E_INVALIDARG;
    }

    FILEDESCRIPTOR* value = file.value();

    if (!value)
    {
        return E_POINTER;
    }

    if (value->dwFlags & FD_ATTRIBUTES
        && value->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return handleDir(file);
    }

    TRACE("Copying %S file\n", file.name());
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
                hr = handle(stm, file);
            }
        }
        break;
    case TYMED_ISTREAM:
        hr = handle(stg.pstm, file);
        break;
    case TYMED_ISTORAGE:
        hr = handle(stg.pstg, file);
        break;
    default:
        hr = DV_E_TYMED;
        break;
    }

    if (SUCCEEDED(hr))
    {
        hr = updateItemTimes(file);
    }

    ReleaseStgMedium(&stg);

    return hr;
}

HRESULT FileDescriptorProcessor::updateItemTimes(FileDescriptorIterator& file)
{
    HRESULT hr = S_OK;

    FILEDESCRIPTOR* value = file.value();

    if (!value)
    {
        return E_POINTER;
    }

    FILETIME *c(0),*a(0),*m(0);
    if (value->dwFlags & FD_CREATETIME)
    {
        c = &value->ftCreationTime;
    }
    if (value->dwFlags & FD_ACCESSTIME)
    {
        a = &value->ftLastAccessTime;
    }
    if (value->dwFlags & FD_WRITESTIME)
    {
        m = &value->ftLastWriteTime;
    }
    if (c || a || m)
    {
        MyStringW fullName = dir().root() / file.name();
        HANDLE h = CreateFile(fullName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
        if (h != INVALID_HANDLE_VALUE)
        {
            SetFileTime(h, c, a, m);

            hr = HRESULT_FROM_WIN32(GetLastError());

            CloseHandle(h);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

HRESULT FileDescriptorProcessor::handle(IStorage* stg, FileDescriptorIterator& file)
{
    HRESULT hr = S_OK;

    MyStringW s = dir().root() / file.name();

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
            /** @todo Ask user what to do with existing file bug #2*/
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

HRESULT FileDescriptorProcessor::handle(IStream* stm, FileDescriptorIterator& file)
{
    HRESULT hr = S_OK;
    MyStringW s = dir().root() / file.name();

    ShPtr<IStream> stmOut;

    hr = SHCreateStreamOnFileW(s, STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &stmOut);

    if (FAILED(hr))
    {
        TRACE("SHCreateStreamOnFileW failed\n");
        DUMPERROR(hr);

        return hr;
    }

    LARGE_INTEGER seekTo={0};
    // At least Microsoft zipfldr.dll does not
    // implment IStream::Seek function so
    // we do not treat E_NOTIMPL as fatal error and proceed
    hr = stm->Seek(seekTo, STREAM_SEEK_SET, NULL);
    if (FAILED(hr) && hr != E_NOTIMPL)
    {
        TRACE("IStream::Seek failed\n");
        DUMPERROR(hr);

        return hr;
    }
    stmOut->Seek(seekTo, STREAM_SEEK_SET, NULL);

    STATSTG stg;
    hr = stm->Stat(&stg, STATFLAG_NONAME);
    ULARGE_INTEGER size = stg.cbSize;
    if (FAILED(hr))
    {
        // Failure getting stream size using IStream::Stat function
        // is not fatal. We can try to determine the value
        // using FILDESCRIPTOR::nFileSize(Low|High) fields.
        // If the fields are not filled this is the problem.
        if (hr == E_NOTIMPL)
        {
            FILEDESCRIPTOR* value = file.value();

            if (!value)
            {
                return E_POINTER;
            }

            if ((value->dwFlags & FD_FILESIZE))
            {
                size.LowPart = value->nFileSizeLow;
                size.HighPart = value->nFileSizeHigh;
            }
        }
        else
        {
            TRACE("IStream::Stat failed\n");
            DUMPERROR(hr);
            return hr;
        }

    }

    /** @todo Implement copying by chuncks of data */
    hr = stm->CopyTo(stmOut, size, NULL, NULL);

    return hr;
}

// vim: set et ts=4 ai :

