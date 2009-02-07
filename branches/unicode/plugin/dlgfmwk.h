/**
 * @file dlgfmwk.h
 * Contains declaration of RunningDialogs class.
 *
 * $Id: dlgfmwk.h 78 2008-11-01 16:57:30Z eleskine $
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
        long param2;
    };
private:
    ActiveDialog* _activeDialog;
    DialogEntry* _head;
    CriticalSection _dialogsLock;
    Message _sent;
    RunningDialogs();
    ~RunningDialogs();
    static void kill(RunningDialogs* p);
    long processPostedSetText(HANDLE dlg, int id, const char* s);
    void processPostedDlgMessages(FarDialog* dlg);
    FarDialog* getDialog(HANDLE handle);
    long processPostedMessage(HANDLE dlg, int msg, int param0, long param1);
public:
    static RunningDialogs* instance();

    void registerDialog(FarDialog* dlg);
    void unregisterDialog(FarDialog* dlg);
    bool lockDialog(FarDialog* dlg);
    void unlockDialog(FarDialog* dlg);

    long sendSafeMessage(HANDLE handle, int msg, int param0, long param1);

    long sendMessage(FarDialog* dlg, int msg, int param0, long param1);
    void postMessage(FarDialog* dlg, int msg, int param0, long param1);

    void notifyDialog(FarDialog* dlg, bool shown);

    long processMessages(Message* msg);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

