
#include <utils.h>

#include "hdrpprcs.h"
#include "configure.hpp"
#include "filelist.h"
#include "filecopy.h"
#include "cpydlg.h"
#include "myshptr.h"
#include <dll_utils.h>

HRESULT HdropProcessor::operator()(IDataObject* obj, DWORD* effect)
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

    Config* config = Config::instance();
    switch (*effect)
    {
    case DROPEFFECT_COPY:
        if (config && config->shellCopy())
        {
            hr = shellCopyOrMoveHDrop(files, false);
        }
        else
        {
            hr = farCopyOrMoveHDropRecursively(files, false);
        }
        break;
    case DROPEFFECT_MOVE:
        *effect = DROPEFFECT_COPY; // the source does not need to delete files
        if (config && config->shellCopy())
        {
            hr = shellCopyOrMoveHDrop(files, true);
        }
        else
        {
            hr = farMoveHDrop(files);
        }
        break;
    case DROPEFFECT_LINK:
        hr = shellLinkHDrop(files);
        break;
    default: // should not happen
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
        DUMPERROR(hr);
    }

    return hr;
}

HRESULT HdropProcessor::farCopyOrMoveHDropRecursively(MyStringW& files, bool move)
{
    /** @todo customize strings in dialog for copy/move operations */
    CopyDialog* dlg = new CopyDialog();
    if (!dlg)
    {
        return E_OUTOFMEMORY;
    }
    dlg->addRef();

    dlg->show(false);
    FileList* list = new FileList(files, dlg);
    if (!list)
    {
        dlg->release();
        return E_OUTOFMEMORY;
    }
    list->addRef();

    FileListEntry e;

    HRESULT hr = S_OK;

    while (hr == S_OK && list->next(e))
    {
        switch (e.type())
        {
        case FileListEntry::File:
            hr = processFile(e, dlg, move);
            break;
        case FileListEntry::EnterDirectory:
            hr = processDir(e);
            break;
        case FileListEntry::LeaveDirectory:
            if (move)
            {
                RemoveDirectory(e.srcpath());
            }
            break;
        }
    }

    dlg->hide();
    dlg->release();
    list->release();

    return hr;
}

HRESULT HdropProcessor::processDir(const FileListEntry& e)
{
    if (!dir().ensureDirectory(e.subpath(), e.data().dwFileAttributes))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT HdropProcessor::processFile(const FileListEntry& e, CopyDialog* dialog, bool move)
{
    MyStringW destPath = dir().root() / e.subpath();

    LARGE_INTEGER size;
    size.LowPart = e.data().nFileSizeLow;
    size.HighPart = e.data().nFileSizeHigh;

    if (dialog && !dialog->nextFile(e.srcpath(), destPath, size.QuadPart))
    {
        return S_FALSE;
    }

    FileCopier copier(e.srcpath(), destPath, dialog, move);
    if (!copier.result())
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

static MyStringW getFileBaseName(const wchar_t* filename, bool stripExtension)
{
    const wchar_t* fileext = nullptr;
    const wchar_t* p;
    for (p = filename; *p; p++)
    {
        if (*p == L'\\')
        {
            filename = p + 1;
            fileext = nullptr;
        }
        else if (*p == L'.')
        {
            fileext = p;
        }
    }
    MyStringW basename = filename;
    if (stripExtension && fileext)
        basename.length(fileext - filename);
    return basename;
}

HRESULT HdropProcessor::farMoveHDrop(MyStringW& files)
{
    // Plan A: call MoveFileEx for every top-level item *without* the flag MOVEFILE_COPY_ALLOWED;
    // if it succeeds, it should be fast, so we don't need progress dialog
    // (the dialog expects recursive scanning of all directories for file sizes,
    //  we want to avoid it unless needed).
    // This handles same-filesystem moves of new files.
    MyStringW failedFiles;
    for (const wchar_t* curItem = files; *curItem; curItem += lstrlen(curItem) + 1)
    {
        BOOL result = MoveFileEx(curItem, dir().root() / getFileBaseName(curItem, false), 0);
        if (!result)
        {
            failedFiles += curItem;
            size_t len = failedFiles.length();
            failedFiles.length(len + 1);
            failedFiles[len] = 0;
        }
    }
    if (!failedFiles.length())
        return S_OK;

    // Plan B: run recursive MoveFileWithProgress on files *with* the flag MOVEFILE_COPY_ALLOWED,
    // with the progress dialog as for copy operation.
    // This handles cross-filesystem moves.
    // Same-filesystem moves of existing files/folders also go here, although they are handled not optimally.
    return farCopyOrMoveHDropRecursively(failedFiles, true);
}

HRESULT HdropProcessor::shellCopyOrMoveHDrop(MyStringW& files, bool move)
{
    SHFILEOPSTRUCT op;
    
    op.hwnd = NULL;
    op.wFunc = move ? FO_MOVE : FO_COPY;
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

/**
 * @brief Converts HDROP into zero-terminated array of zero-terminated strings
 *
 * @param[out] s MyStringW object instance to hold array of strings
 * @param[in] hDrop Handle to files information
 */
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
            int n = MultiByteToWideChar(CP_ACP, 0, p0, -1, p1,
                    static_cast<int>(size));
            p1 += n;
            size -= n;
            p0 += lstrlenA(p0) + 1;
        }

        *p1 = L'\0';
    }

    GlobalUnlock(hDrop);

    return true;
}

HRESULT HdropProcessor::shellLinkHDrop(MyStringW& files)
{
    for (const wchar_t* curItem = files; *curItem; curItem += lstrlenW(curItem) + 1)
    {
        HRESULT hr;
        DWORD attr = GetFileAttributesW(curItem);
        if (attr == INVALID_FILE_ATTRIBUTES)
        {
            hr = E_FAIL;
        }
        else
        {
            ShPtr<IShellLink> shellLink;
            ShPtr<IPersistFile> shellLinkFile;
            hr = CoCreateInstance(
                CLSID_ShellLink,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IShellLink,
                reinterpret_cast<void**>(&shellLink));
            if (SUCCEEDED(hr))
            {
                shellLink->SetPath(curItem);
                //shellLink->SetDescription(L"Shortcut created by Far drag-n-drop plugin");
                hr = shellLink->QueryInterface(IID_IPersistFile,
                    reinterpret_cast<void**>(&shellLinkFile));
            }
            if (SUCCEEDED(hr))
            {
                MyStringW basename = getFileBaseName(curItem, true);
                MyStringW target;
                bool nameOk = false;
                for (int attempt = 1; attempt < 10; attempt++)
                {
                    target = dir().root();
                    target /= basename;
                    wchar_t suffixBuf[16];
                    wchar_t* suffix = suffixBuf;
                    if (attempt > 1)
                    {
                        *suffix++ = L' ';
                        *suffix++ = L'(';
                        *suffix++ = attempt + L'0';
                        *suffix++ = L')';
                    }
                    lstrcpyW(suffix, L".lnk");
                    target += suffixBuf;
                    DWORD attr = GetFileAttributesW(target);
                    if (attr == INVALID_FILE_ATTRIBUTES /*&& GetLastError() == ERROR_FILE_NOT_FOUND*/)
                    {
                        nameOk = true;
                        break;
                    }
                }
                if (!nameOk)
                {
                    SetLastError(ERROR_HANDLE_DISK_FULL);
                    hr = E_FAIL;
                }
                else
                {
                    hr = shellLinkFile->Save(target, TRUE);
                }
            }
        }
        if (FAILED(hr))
        {
            MessageBox(NULL, getLastErrorDesc(), curItem, MB_ICONERROR);
            return hr;
        }
    }
    return S_OK;
}

// vim: set et ts=4 ai :

