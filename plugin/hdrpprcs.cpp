// $Id$

#include "hdrpprcs.h"
#include "utils.h"
#include "configure.hpp"
#include "filelist.h"
#include "dlgnotfy.h"
#include "filecopy.h"
#include "cpydlg.h"

HRESULT HdropProcessor::operator()(IDataObject* obj, DWORD* /*effect*/)
{
    FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM mdm;
    HRESULT hr = obj->GetData(&fmt, &mdm);

    if (FAILED(hr))
    {
        DUMPERROR(hr);
        return hr;
    }

    MyStringW files;
    bool success = initStringInfo(files, mdm.hGlobal);
    ReleaseStgMedium(&mdm);

    if (!success)
    {
        return E_FAIL;
    }

    if (Config::instance()->shellCopy())
        hr = shellCopyHDrop(files);
    else
        hr = farCopyHDrop(files);

    if (FAILED(hr))
    {
        DUMPERROR(hr);
    }

    return hr;
}

HRESULT HdropProcessor::farCopyHDrop(MyStringW& files)
{
    CopyDialog dlg;
    dlg.show(false);
    CopyDialogNotify n(this, &dlg);
    FileList list(files, &n);
    FileListEntry e;

    while (list.next(e))
    {
        HRESULT hr;
        if ((e.data().dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ==
                FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = processDir(e);
        }
        else
        {
            hr = processFile(e, &n);
        }

        if (hr != S_OK)
            return hr;
    }
    return S_OK;
}

HRESULT HdropProcessor::processDir(const FileListEntry& e)
{
    if (!dir().ensureDirectory(e.subpath()))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT HdropProcessor::processFile(const FileListEntry& e, CopyDialogNotify* n)
{
    MyStringW destPath = dir().root() / e.subpath();

    LARGE_INTEGER size;
    size.LowPart = e.data().nFileSizeLow;
    size.HighPart = e.data().nFileSizeHigh;

    if (n && n->dialog() && !n->dialog()->nextFile(e.srcpath(), destPath, size.QuadPart))
        return S_FALSE;

    FileCopier copier(e.srcpath(), destPath, n);
    if (!copier.result())
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT HdropProcessor::shellCopyHDrop(MyStringW& files)
{
    SHFILEOPSTRUCT op;
    
    op.hwnd = NULL;
    op.wFunc = FO_COPY;
    op.pFrom = files;
    op.pTo = dir().root();
    op.fFlags = 0;
    op.fAnyOperationsAborted = FALSE;
    op.hNameMappings = NULL;
    op.lpszProgressTitle = NULL;

    if (!SHFileOperation(&op))
        return S_OK;

    LASTERROR();
    return HRESULT_FROM_WIN32(GetLastError());
}

bool HdropProcessor::initStringInfo(MyStringW& s, HGLOBAL hDrop)
{
    size_t size = GlobalSize(hDrop);

    if (!size)
        return false;

    DROPFILES* files = (DROPFILES*)GlobalLock(hDrop);

    size -= files->pFiles;
    
    s.length(size);
    if (files->fWide)
    {
        memmove(s, (char*)files + files->pFiles, size);
    }
    else
    {
        wchar_t* p1 = s;
        char* p0 = (char*)files + files->pFiles;

        while (*p0)
        {
            int n = MultiByteToWideChar(CP_ACP, 0, p0, -1, p1, size);
            p1 += n;
            size -= n;
            p0 += lstrlenA(p0) + 1;
        }

        *p1 = L'\0';
    }

    GlobalUnlock(hDrop);

    return true;
}

// vim: set et ts=4 ai :

