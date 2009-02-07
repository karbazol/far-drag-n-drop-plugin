/**
 * @file cperrdlg.cpp
 * This file contains implementation of CopyErrorDialog class.
 *
 * $Id: cperrdlg.cpp 78 2008-11-01 16:57:30Z eleskine $
 */

#include "cperrdlg.h"
#include "ddlng.h"

CopyErrorDialog::CopyErrorDialogItems CopyErrorDialog::itemsTemplate =
{
    {DI_DOUBLEBOX,3,1,34,8,0,0,0,0,(char*)MError},
    {DI_TEXT,5,2,32,0,0,0,DIF_CENTERGROUP,0,""},
    {DI_TEXT,5,3,32,0,0,0,DIF_CENTERGROUP,0,(char*)MCopyError},
    {DI_TEXT,5,4,32,0,0,0,DIF_CENTERGROUP,0,""},
    {DI_TEXT,5,5,32,0,0,0,DIF_CENTERGROUP,0,(char*)MCopyingTo},
    {DI_TEXT,5,6,32,0,0,0,DIF_CENTERGROUP,0,""},
    // -- buttons --
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char*)MRetry},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char*)MSkip},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char*)MSkipall},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char*)MCancel}
};

InitDialogItem* CopyErrorDialog::items()
{
    return &itemsTemplate.frame;
}

int CopyErrorDialog::itemsCount()
{
    return sizeof(CopyErrorDialogItems)/sizeof(InitDialogItem);
}

CopyErrorDialog::RetCode CopyErrorDialog::show(const wchar_t* /*source*/, const wchar_t* /*dest*/, unsigned int /*error*/)
{
    FarDialog::show(true);
    return cancel;
}

DWORD CopyErrorDialog::flags()
{
    return FDLG_WARNING;
}

// vim: set et ts=4 sw=4 ai :

