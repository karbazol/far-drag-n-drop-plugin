#ifndef __KARBAZOL_DRAGNDROP_2_0__CPERRDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__CPERRDLG_H__

/**
 * @file cperrdlg.h
 * This file contains declaration of CopyErrorDialog class.
 *
 * $Id$
 */

#include "fardlg.h"

class CopyErrorDialog : public FarDialog
{
private:
    struct CopyErrorDialogItems
    {
        InitDialogItem frame;            //  Error                                       
        // InitDialogItem sysErrMessage; //  File sharing violation (this member has been moved to the end of structure)
        InitDialogItem errMessage;       //  Cannot copy file
        InitDialogItem srcFileName;      //  only file name without the path
        InitDialogItem lblTo;            //  to
        InitDialogItem dstFileName;      //  target directory plus path
        InitDialogItem btnRetry; InitDialogItem btnSkip; InitDialogItem btnSkipAll; InitDialogItem btnCancel; // those are the buttons
        InitDialogItem sysErrMessage[1];
    };
    static CopyErrorDialogItems itemsTemplate;
    InitDialogItem* _items;
    int _itemsCount;
    MyStringW _errorMessage;
    MyStringW _srcFileName;
    MyStringW _dstFileName;
    GrowOnlyArray<MyStringW> _errorLines;
    void allocItems(size_t additionalErrorLines);
protected:
    void prepareItems(int consoleWidth, int consoleHeight);
    InitDialogItem* items();
    void releaseItems();
    int itemsCount();
    DWORD flags();
public:
    enum RetCode
    {
        retry = 0,
        skip,
        skipAll,
        cancel
    };
    CopyErrorDialog(): FarDialog(), _items(0), _itemsCount(0), _errorMessage(),
        _srcFileName(), _dstFileName(), _errorLines(){}
    ~CopyErrorDialog(){}
    RetCode show(const wchar_t* source, const wchar_t* dest, unsigned int error);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__CPERRDLG_H__
// vim: set et ts=4 sw=4 ai :

