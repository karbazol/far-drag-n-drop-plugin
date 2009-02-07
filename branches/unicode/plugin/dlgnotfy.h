/**
 * @file dlgnotfy.h
 * Contains declaration of CopyDialogNotify class.
 *
 * $Id: dlgnotfy.h 61 2008-05-11 15:47:03Z eleskine $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DLGNOTFY_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLGNOTFY_H__

#include "filelist.h"
#include "filecopy.h"

class HdropProcessor;
class CopyDialog;

/**
 * @brief Class to receive notifications from FileList
 *
 * Used during directory scan to deliver notification from
 * FileList objects to specified CopyDialog instance
 */
class CopyDialogNotify : public FileListNotify, public FileCopier::FileCopyNotify
{
private:
    HdropProcessor* _processor;
    CopyDialog* _dialog;
    bool onNextEntry(const int reason, const FileListEntry& e);
    bool onAllProcessed();
    bool onFileExists(const wchar_t* src, const wchar_t* dest);
    bool onFileStep(const __int64& step);
    bool onFileError(const wchar_t* src, const wchar_t* dest, DWORD errorNumber);
public:
    CopyDialogNotify(HdropProcessor* p, CopyDialog* dlg): _processor(p), _dialog(dlg){}
    ~CopyDialogNotify(){}
    inline CopyDialog* dialog(){return _dialog;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGNOTFY_H__

