/**
 * @file cperrdlg.cpp
 * This file contains implementation of CopyErrorDialog class.
 *
 * $Id$
 */

#include "dll_utils.h"
#include "cperrdlg.h"
#include "ddlng.h"

/**
 * Utility macro used to determine id of dialog item
 */
#define getMyItemId(i) getFarDlgItemId(CopyErrorDialogItems,i)

CopyErrorDialog::CopyErrorDialogItems CopyErrorDialog::itemsTemplate =
{
    {DI_DOUBLEBOX,3,1,34,8,0,0,0,0,(wchar_t*)MError},
    {DI_TEXT,5,2,32,0,0,0,DIF_CENTERGROUP,0,L""},
    {DI_TEXT,5,3,32,0,0,0,DIF_CENTERGROUP,0,(wchar_t*)MCopyError},
    {DI_TEXT,5,4,32,0,0,0,DIF_CENTERGROUP,0,L""},
    {DI_TEXT,5,5,32,0,0,0,DIF_CENTERGROUP,0,(wchar_t*)MCopyingTo},
    {DI_TEXT,5,6,32,0,0,0,DIF_CENTERGROUP,0,L""},
    // -- buttons --
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MRetry},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MSkip},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MSkipall},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MCancel}
};

void CopyErrorDialog::calcWidth()
{
    /** @todo Adjust dialog width  bug #2.*/ 
    int minWidth = itemsTemplate.frame.X2 - 6;

    int width = max(minWidth,
            max(lstrlen(_items.sysErrMessage.Data),
                max(lstrlen(_items.srcFileName.Data),
                    lstrlen(_items.dstFileName.Data))));

    _items.frame.X2 = width + 6;
    /** @todo Wrap sysErrorMessage if its length exceeds maximum allowed width */
    _items.sysErrMessage.X2 = _items.srcFileName.X2 = _items.dstFileName.X2 = width;
}

InitDialogItem* CopyErrorDialog::items()
{
    return &_items.frame;
}

int CopyErrorDialog::itemsCount()
{
    return sizeof(CopyErrorDialogItems)/sizeof(InitDialogItem);
}

CopyErrorDialog::RetCode CopyErrorDialog::show(const wchar_t* source, const wchar_t* dest, unsigned int error)
{
    MyStringW desc = getErrorDesc(error);
    _items.sysErrMessage.Data = desc; 
    _items.srcFileName.Data = source;
    _items.dstFileName.Data = dest;

    return static_cast<RetCode>(FarDialog::show(true) - getMyItemId(btnRetry));
}

DWORD CopyErrorDialog::flags()
{
    return FDLG_WARNING;
}

// vim: set et ts=4 sw=4 ai :

