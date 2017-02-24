/**
 * @file dlgfmwk.h
 * Contains declaration of RunningDialogs class.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

#include <common/ddlock.h>

#include "fardlg.h"

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
        intptr_t message;
        intptr_t param1;
        void* param2;
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
    intptr_t processPostedSetText(HANDLE dlg, intptr_t id, const wchar_t* s);
    void processPostedDlgMessages(FarDialog* dlg);
    FarDialog* getDialog(HANDLE handle);
    intptr_t processPostedMessage(HANDLE dlg, intptr_t msg, intptr_t param0, void* param1);
public:
    static RunningDialogs* instance();

    void registerDialog(FarDialog* dlg);
    void unregisterDialog(FarDialog* dlg);
    bool lockDialog(FarDialog* dlg);
    void unlockDialog(FarDialog* dlg);

    intptr_t sendSafeMessage(HANDLE handle, intptr_t msg, intptr_t param0, void* param1);

    intptr_t sendMessage(FarDialog* dlg, intptr_t msg, intptr_t param0, void* param1);
    void postMessage(FarDialog* dlg, intptr_t msg, intptr_t param0, void* param1);

    void notifyDialog(FarDialog* dlg, bool shown);

    LONG_PTR processMessages(Message* msg);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGFMWK_H__

