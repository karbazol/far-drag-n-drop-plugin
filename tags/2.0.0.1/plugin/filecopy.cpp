// $Id: filecopy.cpp 26 2008-04-20 18:48:32Z eleskine $

#include "filecopy.h"

FileCopier::FileCopier(const wchar_t* src, const wchar_t* dest, FileCopyNotify* p):
    _src(src), _dest(dest), _notify(p), _copied(0)
{
    _result = doCopy();
}

bool FileCopier::doCopy()
{
    BOOL cancel = FALSE;
    return !!CopyFileEx(
            _src, _dest, reinterpret_cast<LPPROGRESS_ROUTINE>(&winCallBack),
            this, &cancel, 0);
}

DWORD FileCopier::winCallBack(LARGE_INTEGER /*totalSize*/, LARGE_INTEGER transferred,
            LARGE_INTEGER /*streamSize*/, LARGE_INTEGER /*streamBytesTransferred*/,
            DWORD /*streamNumber*/, DWORD /*dwCallBackReason*/, HANDLE /*hSourcefile*/,
            HANDLE /*hDestinationFile*/, FileCopier* This)
{
    __int64 step = transferred.QuadPart - This->_copied;
    This->_copied = transferred.QuadPart;
    if (This->_notify && !This->_notify->onFileStep(step))
        return PROGRESS_CANCEL;
    return PROGRESS_CONTINUE;
}

// vim: set et ts=4 ai :
