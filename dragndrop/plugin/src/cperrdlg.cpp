/**
 * @file cperrdlg.cpp
 * This file contains implementation of CopyErrorDialog class.
 *
 */

#include <dll/dll_utils.h>
#include "cperrdlg.h"
#include "ddlng.h"
#include "dndguids.h"

/**
 * Utility macro used to determine id of dialog item
 */
#define getMyItemId(i) getFarDlgItemId(CopyErrorDialogItems,i)

CopyErrorDialog::CopyErrorDialogItems CopyErrorDialog::itemsTemplate =
{
    {DI_DOUBLEBOX,3,1,34,8,0,0,0,0,(wchar_t*)MError},
    //{DI_TEXT,5,2,32,0,0,0,DIF_CENTERGROUP,0,L""}, // this member has been moved to the end of structure
    {DI_TEXT,5,3,32,0,0,0,DIF_CENTERGROUP,0,(wchar_t*)MCopyError},
    {DI_TEXT,5,4,32,0,0,0,DIF_CENTERGROUP,0,L""},
    {DI_TEXT,5,5,32,0,0,0,DIF_CENTERGROUP,0,(wchar_t*)MCopyingTo},
    {DI_TEXT,5,6,32,0,0,0,DIF_CENTERGROUP,0,L""},
    // -- buttons --
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MRetry},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MSkip},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MSkipall},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(wchar_t*)MCancel},
    {DI_TEXT,5,2,32,0,0,0,DIF_CENTERGROUP,0,L""}, // this member has been moved to the end of structure
};

const GUID& CopyErrorDialog::Id() const
{
    return copyErrDialogGuid;
}

void CopyErrorDialog::allocItems(size_t additionalErrorLines)
{
    _itemsCount = sizeof(CopyErrorDialogItems)/sizeof(InitDialogItem)
        + static_cast<int>(additionalErrorLines);

    _items = reinterpret_cast<InitDialogItem*>(
            ::malloc(sizeof(InitDialogItem)*(_itemsCount)));
    if (!_items)
    {
        _itemsCount = 0;
    }
}

void CopyErrorDialog::prepareItems(int consoleWidth, int consoleHeight)
{
    consoleHeight;

    if (!_items)
    {
        return;
    }

    int maxStrLen = consoleWidth - 8;
    _srcFileName = TruncPathStr(_srcFileName, maxStrLen);
    _dstFileName = TruncPathStr(_dstFileName, maxStrLen);
    wrapString(_errorMessage, 
            min(_srcFileName.length(), _dstFileName.length()), _errorLines);

    int extraSize = static_cast<int>(_errorLines.size()) - 1;
    allocItems(extraSize);

    CopyErrorDialogItems& items(*reinterpret_cast<CopyErrorDialogItems*>(_items));

    items = itemsTemplate;

    items.btnRetry.Data = reinterpret_cast<const wchar_t*>(_useOverwrite ? MCopyOverwrite : MRetry);

    items.srcFileName.Data = _srcFileName;
    items.dstFileName.Data = _dstFileName;

    items.sysErrMessage[0].Data = _errorLines[0];

    int minWidth = itemsTemplate.frame.X2 - 6;

    int width = max(minWidth,
                    max(lstrlen(items.srcFileName.Data),
                        lstrlen(items.dstFileName.Data)));

    items.frame.X2 = width + 6;
    
    items.sysErrMessage[0].X2 = items.srcFileName.X2 = items.dstFileName.X2 = width;

    if (extraSize)
    {
        int i;
        for (i = getMyItemId(errMessage); i < getMyItemId(sysErrMessage); ++i)
        {
            _items[i].Y2 = _items[i].Y1 += extraSize;
        }
    
        items.frame.Y2 += extraSize;

        InitDialogItem* errLines = items.sysErrMessage + 1;
        for (i = 0; i < extraSize; ++i)
        {
            errLines[i] = *items.sysErrMessage;
            errLines[i].Y2 = errLines[i].Y1 += i+1;
            errLines[i].Data = _errorLines[i+1];
        }
    }
}

void CopyErrorDialog::releaseItems()
{
    if (_items)
    {
        ::free(_items);
        _items = 0;
        _itemsCount = 0;
    }
}

InitDialogItem* CopyErrorDialog::items()
{
    return _items;
}

size_t CopyErrorDialog::itemsCount()
{
    return _itemsCount;
}

CopyErrorDialog::RetCode CopyErrorDialog::show(const wchar_t* source, const wchar_t* dest, unsigned int error)
{
    _errorMessage = getErrorDesc(error);
    _srcFileName = source;
    _dstFileName = dest;

    return static_cast<RetCode>(FarDialog::show(true) - getMyItemId(btnRetry));
}

DWORD CopyErrorDialog::flags()
{
    return FDLG_WARNING;
}

// vim: set et ts=4 sw=4 ai :

