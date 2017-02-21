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
        FAR_WPARAM_TYPE message;
        FAR_WPARAM_TYPE param1;
        FAR_LPARAM_TYPE param2;
        Message(): h(0), message(0), param1(0), param2(0){}
    };
private:
    ActiveDialog* _activeDialog;
    DialogEntry* _head;
    CriticalSection _dialogsLock;
    Message _sent;
    RunningDialogs();
    ~RunningDialogs();
    static void kill(RunningDialogs* p);
    FAR_RETURN_TYPE processPostedSetText(HANDLE dlg, FAR_WPARAM_TYPE id, const wchar_t* s);
    void processPostedDlgMessages(FarDialog* dlg);
    FarDialog* getDialog(HANDLE handle);
    FAR_RETURN_TYPE processPostedMessage(HANDLE dlg, FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param0, FAR_LPARAM_TYPE param1);
public:
    static RunningDialogs* instance();

    void registerDialog(FarDialog* dlg);
    void unregisterDialog(FarDialog* dlg);
    bool lockDialog(FarDialog* dlg);
    void unlockDialog(FarDialog* dlg);

    FAR_RETURN_TYPE sendSafeMessage(HANDLE handle, FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param0, FAR_LPARAM_TYPE param1);

    FAR_RETURN_TYPE sendMessage(FarDialog* dlg, FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param0, FAR_LPARAM_TYPE param1);
    void postMessage(FarDialog* dlg, FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param0, FAR_LPARAM_TYPE param1);

    void notifyDialog(FarDialog* dlg, bool shown);

    FAR_RETURN_TYPE processMessages(Message* msg);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

