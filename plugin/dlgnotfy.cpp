// $Id$

#include "hdrpprcs.h"
#include "cpydlg.h"
#include "dlgnotfy.h"

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

// vim: set et ts=4 ai :

