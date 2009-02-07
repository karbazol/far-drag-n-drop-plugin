#include <windows.h>
#include <shlobj.h>
#include "shutils.h"
#include "myshptr.h"
#include "utils.h"

HRESULT PathToIDList(unsigned int /*cp*/, LPCITEMIDLIST /*parent*/, const char* /*path*/, LPITEMIDLIST* /*pidl*/)
{
    ShPtr<IShellFolder> desktop;

    HRESULT res;
    res = SHGetDesktopFolder(&desktop);
    if (FAILED(res))
        return res;

    return res;
}

HRESULT ConsolePathToIDList(LPCITEMIDLIST parent, const char* path, LPITEMIDLIST* pidl)
{
    return PathToIDList(GetConsoleCP(), parent, path, pidl);
}

static void CopyMediumIStream(STGMEDIUM& dest, const STGMEDIUM& src)
{
    CreateStreamOnHGlobal(0, TRUE, &dest.pstm);
    
    STATSTG stat;
    if (FAILED(src.pstm->Stat(&stat, STATFLAG_NONAME)))
        return;

    LARGE_INTEGER zero={0,0};
    dest.pstm->Seek(zero, STREAM_SEEK_SET, NULL);
    src.pstm->Seek(zero, STREAM_SEEK_SET, NULL);

    src.pstm->CopyTo(dest.pstm, stat.cbSize, NULL, NULL);

    dest.pstm->Seek(zero, STREAM_SEEK_SET, NULL);
    src.pstm->Seek(zero, STREAM_SEEK_SET, NULL);
}

void CopyMedium(STGMEDIUM& dest, const STGMEDIUM& src, CLIPFORMAT cf)
{
    switch (src.tymed)
    {
    case TYMED_HGLOBAL:
    case TYMED_GDI:
    case TYMED_MFPICT:
    case TYMED_ENHMF:
        dest.hGlobal = OleDuplicateData(src.hGlobal, cf, GHND|GMEM_SHARE);
        break;
    case TYMED_FILE:
        {
            dest.lpszFileName = (LPOLESTR)CoTaskMemAlloc((lstrlenW(dest.lpszFileName)+1)*sizeof(*dest.lpszFileName));
            lstrcpyW(dest.lpszFileName, src.lpszFileName);
        }
        break;
    case TYMED_ISTREAM:
        CopyMediumIStream(dest, src);
        break;
    case TYMED_ISTORAGE:
        {
            dest.pstg = src.pstg;
            if (dest.pstg)
            {
                dest.pstg->AddRef();
            }
        }
        break;
    case TYMED_NULL:
    default:
        break;
    }
    dest.tymed = src.tymed;
    dest.pUnkForRelease = src.pUnkForRelease;
}

#define DEFINE_CBD_FMT(x)                           \
CLIPFORMAT get_##x()                                \
{                                                   \
    static CLIPFORMAT res = (CLIPFORMAT)0;          \
    if (!res)                                       \
        res = (CLIPFORMAT)                          \
            RegisterClipboardFormat(CFSTR_##x);     \
    return res;                                     \
}                                                   \

DEFINE_CBD_FMT(DRAGCONTEXT)
DEFINE_CBD_FMT(DRAGIMAGEBITS)
DEFINE_CBD_FMT(LOGICALPERFORMEDDROPEFFECT)
DEFINE_CBD_FMT(PREFERREDDROPEFFECT)
DEFINE_CBD_FMT(PERFORMEDDROPEFFECT)
DEFINE_CBD_FMT(PASTESUCCEEDED)
DEFINE_CBD_FMT(TARGETCLSID)
DEFINE_CBD_FMT(UNTRUSTEDDRAGDROP)
DEFINE_CBD_FMT(FILEDESCRIPTORA)
DEFINE_CBD_FMT(FILEDESCRIPTORW)
DEFINE_CBD_FMT(FILECONTENTS)
DEFINE_CBD_FMT(SHELLIDLIST)
DEFINE_CBD_FMT(SHELLIDLISTOFFSET)
DEFINE_CBD_FMT(INDRAGLOOP)

// vim: set et ts=4 ai :

