// $Id$

#include "hdrpprcs.h"
#include "cpydlg.h"
#include "dlgnotfy.h"
#include "cperrdlg.h"

bool CopyDialogNotify::onNextEntry(const int /*reason*/, const FileListEntry& e)
{
    if (!_dialog)
        return true;

    LARGE_INTEGER size;
    size.LowPart  = e.data().nFileSizeLow;
    size.HighPart = e.data().nFileSizeHigh;

    if (FILE_ATTRIBUTE_DIRECTORY != (e.data().dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            && !_dialog->appendFile(size.QuadPart, false))
        return false;

    return true;
}

bool CopyDialogNotify::onAllProcessed()
{
    if (!_dialog)
        return true;

    if (!_dialog->appendFile(0, true))
        return false;

    return true;
}

bool CopyDialogNotify::onFileExists(const wchar_t* src, const wchar_t* dest)
{
    src;
    dest;

    /** @todo Ask user whether to overwrite files bug #3 */
    return true;
}

bool CopyDialogNotify::onFileStep(const __int64& step)
{
    if (!_dialog)
        return true;

    if (!_dialog->step(step))
        return false;

    return true;
}

bool CopyDialogNotify::onFileError(const wchar_t* src, const wchar_t* dest, DWORD errorNumber)
{
    /** @todo Implement error check */
    if (_dialog && _dialog->running())
    {
        // give the _dialog a chance to appear
        while (_dialog->sendMessage(DM_GETTEXTPTR, 0, 0) == 0)
        {
            Sleep(1);
        }
    }
    CopyErrorDialog dlg;
    dlg.show(src, dest, errorNumber);
    return false;
}

// vim: set et ts=4 ai :

