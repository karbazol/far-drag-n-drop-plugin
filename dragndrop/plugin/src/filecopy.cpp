#include "filecopy.h"

FileCopier::FileCopier(const wchar_t* src, const wchar_t* dest, FileCopyNotify* p, bool move):
    _src(src), _dest(dest), _notify(p), _copied(0)
{
    if (_notify)
    {
        _notify->addRef();
    }
    _result = move ? doMove() : doCopy();
}

FileCopier::~FileCopier()
{
    if (_notify)
    {
        _notify->release();
        _notify = 0;
    }
}

bool FileCopier::doCopy()
{
    do
    {
        BOOL cancel = FALSE;
        if (CopyFileEx(
            _src, _dest, &winCallBack,
            this, &cancel, 0|
            COPY_FILE_FAIL_IF_EXISTS|
            0
            ) || cancel)
        {
            return true;
        }
        DWORD err = GetLastError();
        LASTERROR();

    } while (_notify && _notify->onFileError(_src, _dest, GetLastError()));

    return false;
}

bool FileCopier::doMove()
{
    do
    {
        if (MoveFileWithProgress(_src, _dest, &winCallBack, this, MOVEFILE_COPY_ALLOWED)
            || GetLastError() == ERROR_REQUEST_ABORTED)
        {
            return true;
        }
        DWORD err = GetLastError();
        LASTERROR();
    } while (_notify && _notify->onFileError(_src, _dest, GetLastError()));

    return false;
}

DWORD FileCopier::winCallBack(LARGE_INTEGER /*totalSize*/, LARGE_INTEGER transferred,
            LARGE_INTEGER /*streamSize*/, LARGE_INTEGER /*streamBytesTransferred*/,
            DWORD /*streamNumber*/, DWORD /*dwCallBackReason*/, HANDLE /*hSourcefile*/,
            HANDLE /*hDestinationFile*/, PVOID param)
{
    FileCopier* This = reinterpret_cast<FileCopier*>(param);
    int64_t step = transferred.QuadPart - This->_copied;
    This->_copied = transferred.QuadPart;
    if (This->_notify && !This->_notify->onFileStep(step))
    {
        return PROGRESS_CANCEL;
    }

    return PROGRESS_CONTINUE;
}

// vim: set et ts=4 ai :

