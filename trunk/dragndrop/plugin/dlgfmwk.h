/**
 * @file dlgfmwk.h
 * Contains declaration of RunningDialogs class.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

#include "fardlg.h"
#include "ddlock.h"

class DialogEntry;
class ActiveDialog;

/**
 * @brief Main Dialog Framework class
 *
 * Represents a manager of running Far dialogs related to the plug-in
 */
class RunningDialogs
{
public:
    struct Message
    {
        HANDLE h;
        int message;
        int param1;
        LONG_PTR param2;
    };
private:
    ActiveDialog* _activeDialog;
    DialogEntry* _head;
    CriticalSection _dialogsLock;
    Message _sent;
    RunningDialogs();
    ~RunningDialogs();
    static void kill(RunningDialogs* p);
    LONG_PTR processPostedSetText(HANDLE dlg, int id, const wchar_t* s);
    void processPostedDlgMessages(FarDialog* dlg);
    FarDialog* getDialog(HANDLE handle);
    LONG_PTR processPostedMessage(HANDLE dlg, int msg, int param0, LONG_PTR param1);
public:
    static RunningDialogs* instance();

    void registerDialog(FarDialog* dlg);
    void unregisterDialog(FarDialog* dlg);
    bool lockDialog(FarDialog* dlg);
    void unlockDialog(FarDialog* dlg);

    LONG_PTR sendSafeMessage(HANDLE handle, int msg, int param0, LONG_PTR param1);

    LONG_PTR sendMessage(FarDialog* dlg, int msg, int param0, LONG_PTR param1);
    void postMessage(FarDialog* dlg, int msg, int param0, LONG_PTR param1);

    void notifyDialog(FarDialog* dlg, bool shown);

    LONG_PTR processMessages(Message* msg);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

