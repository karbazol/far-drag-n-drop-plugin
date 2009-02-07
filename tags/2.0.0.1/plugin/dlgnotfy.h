// $Id: dlgnotfy.h 26 2008-04-20 18:48:32Z eleskine $
#ifndef __KARBAZOL_DRAGNDROP_2_0__DLGNOTFY_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLGNOTFY_H__

#include "filelist.h"
#include "filecopy.h"

class HdropProcessor;
class CopyDialog;

class CopyDialogNotify : public FileListNotify, public FileCopier::FileCopyNotify
{
private:
    HdropProcessor* _processor;
    CopyDialog* _dialog;
    bool onNextEntry(const int reason, const FileListEntry& e);
    bool onAllProcessed();
    bool onFileExists(const wchar_t* src, const wchar_t* dest);
    bool onFileStep(const __int64& step);
public:
    CopyDialogNotify(HdropProcessor* p, CopyDialog* dlg): _processor(p), _dialog(dlg){}
    ~CopyDialogNotify(){}
    inline CopyDialog* dialog(){return _dialog;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGNOTFY_H__

