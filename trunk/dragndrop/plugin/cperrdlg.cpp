/**
 * @file cperrdlg.cpp
 * This file contains implementation of CopyErrorDialog class.
 *
 * $Id$
 */

#include "cperrdlg.h"
#include "ddlng.h"

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

InitDialogItem* CopyErrorDialog::items()
{
    return &_items.frame;
}

int CopyErrorDialog::itemsCount()
{
    return sizeof(CopyErrorDialogItems)/sizeof(InitDialogItem);
}

CopyErrorDialog::RetCode CopyErrorDialog::show(const wchar_t* source, const wchar_t* dest, unsigned int /*error*/)
{
    _items.srcFileName.Data = source;
    _items.dstFileName.Data = dest;
    FarDialog::show(true);
    return cancel;
}

DWORD CopyErrorDialog::flags()
{
    return FDLG_WARNING;
}

// vim: set et ts=4 sw=4 ai :

