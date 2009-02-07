// $Id$

#ifndef __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

#include "fardlg.h"
#include "ddlock.h"

class DialogEntry;

class RunningDialogs
{
private:
    DialogEntry* _head;
    CriticalSection _dialogsLock;
    RunningDialogs();
    ~RunningDialogs();
    static void kill(RunningDialogs* p);
    long processPostedSetText(HANDLE dlg, int id, const char* s);
public:
    static RunningDialogs* instance();

    void registerDialog(FarDialog* dlg);
    void unregisterDialog(FarDialog* dlg);
    bool lockDialog(FarDialog* dlg);
    void unlockDialog(FarDialog* dlg);

    FarDialog* getDialog(HANDLE handle);
    long sendSafeMessage(HANDLE handle, int msg, int param0, long param1);

    void postMessage(FarDialog* dlg, int msg, int param0, long param1);
    long processPostedMessage(HANDLE dlg, int msg, int param0, long param1);

    void postMyMessages(FarDialog* dlg);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

