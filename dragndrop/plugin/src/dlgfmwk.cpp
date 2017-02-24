/**
 * @file dlgfmwk.cpp
 * Contains implementation of Dialogs Framework
 *
 */

#include <growarry.h>
#include <dll.h>
#include <mystring.h>

#include "dlgfmwk.h"
#include "mainthrd.h"

intptr_t SendDlgMessage(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2);

/**
 * @brief Dialog entry
 *
 * Dialog entry object used by Dialogs Framework
 */
class DialogEntry
{
private:
    struct ControlTextMessage
    {
        bool queued;
        MyStringW text;
        ControlTextMessage(): queued(false), text(){}
        ControlTextMessage(const ControlTextMessage& m):
            queued(m.queued), text(m.text){}
        ControlTextMessage& operator=(const ControlTextMessage& m)
        {
            queued = m.queued;
            text = m.text;

            return *this;
        }
    };
    typedef GrowOnlyArray<ControlTextMessage> ControlTexts;
    typedef GrowOnlyArray<RunningDialogs::Message> Messages;

    DialogEntry* _next;
    FarDialog* _dlg;
    CriticalSection _lock;
    ControlTexts _texts;
    Messages _messages;
    DialogEntry():_next(0),_dlg(0),_lock(),_texts(),_messages()
    {
    }
    DialogEntry(DialogEntry* head, FarDialog* dlg):
        _next(head), _dlg(dlg),_lock(),_texts(),_messages()
    {
    }
    typedef bool (DialogEntry::*cmpDlg_t)(void* p) const;
    bool cmpDlg(void* dlg) const
    {
        if (!this)
            return false;
        return reinterpret_cast<FarDialog*>(dlg) == _dlg;
    }
    bool cmpHandle(void* h) const
    {
        if (!this || !_dlg)
            return false;
        return reinterpret_cast<HANDLE>(h) == _dlg->hwnd();
    }
    DialogEntry* doFind(void* p, cmpDlg_t c, DialogEntry** pprev)
    {
        DialogEntry* prev = 0;
        DialogEntry* res = this;

        while (res)
        {
            if ((res->*c)(p))
                break;
            prev = res;
            res = res->_next;
        }

        if (pprev)
            *pprev = prev;
        return res;
    }
    // Disable direct deletion
    ~DialogEntry()
    {
    }
public:
    DialogEntry* newEntry(FarDialog* dlg)
    {
        if (!dlg)
            return 0;
        return new DialogEntry(this, dlg);
    }
    inline FarDialog* dialog() const {return _dlg;}
    inline CriticalSection& lock() {return _lock;}

    inline DialogEntry* find(FarDialog* dlg, DialogEntry** prev=NULL)
    {
        return doFind(dlg, &DialogEntry::cmpDlg, prev);
    }

    inline DialogEntry* find(HANDLE h, DialogEntry** prev=NULL)
    {
        return doFind(h, &DialogEntry::cmpHandle, prev);
    }

    void deleteIt(DialogEntry* prev=0)
    {
        if (!this)
            return;

        if (prev && prev->_next == this)
            prev->_next = _next;

        delete this;
    }

    void deleteAll()
    {
        DialogEntry* p = this;

        while (p)
        {
            DialogEntry* n = p->_next;
            delete p;
            p = n;
        }
    }

    bool setControlText(intptr_t id, const wchar_t* s)
    {
        if (!_texts.size())
            _texts.size(_dlg->itemsCount());
        ControlTextMessage& m = _texts[id];
        bool res = m.queued;
        m.text = s;
        m.queued = true;

        return res;
    }

    bool getControlText(intptr_t id, MyStringW& s)
    {
        if (!_texts.size())
            _texts.size(_dlg->itemsCount());
        ControlTextMessage& m = _texts[id];
        bool res = m.queued;
        m.queued = false;

        s = m.text;

        return res;
    }
    RunningDialogs::Message* addMessage(HANDLE h, intptr_t msg, intptr_t p1, void* p2)
    {
        RunningDialogs::Message m;
        m.h = h;
        m.message = msg;
        m.param1 = p1;
        m.param2 = p2;

        return &_messages.append(m);
    }
    inline Messages messages(){return _messages;}
};

class ActiveDialog
{
private:
    ActiveDialog* _next;
    FarDialog* _dlg;
    ActiveDialog(ActiveDialog* next, FarDialog* dlg): _next(next), _dlg(dlg){}
    ~ActiveDialog(){}
public:
    ActiveDialog* push(FarDialog* dlg)
    {
        return new ActiveDialog(this, dlg);
    }
    ActiveDialog* pop()
    {
        if (!this)
            return 0;

        ActiveDialog* res = _next;

        delete this;

        return res;
    }
    void deleteAll()
    {
        for (ActiveDialog* d = this; d != 0; d = d->pop());
    }

    inline FarDialog* dialog() const {return _dlg;}
};

RunningDialogs::RunningDialogs():_activeDialog(0),_head(0), _dialogsLock()
{
}

RunningDialogs::~RunningDialogs()
{
    _activeDialog->deleteAll();
    _head->deleteAll();
}

void RunningDialogs::kill(RunningDialogs* p)
{
    delete p;
}

RunningDialogs* RunningDialogs::instance()
{
    static RunningDialogs* p = 0;

    if (!p)
    {
        p = new RunningDialogs();
        if (p)
        {
            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
            }
        }
    }

    return p;
}

void RunningDialogs::registerDialog(FarDialog* dlg)
{
    LOCKIT(_dialogsLock);
    _head = _head->newEntry(dlg);
}

bool RunningDialogs::lockDialog(FarDialog* dlg)
{
    LOCKIT(_dialogsLock);
    DialogEntry* e = _head->find(dlg);
    if (!e)
        return false;

    e->lock().lock();

    return true;
}

void RunningDialogs::unlockDialog(FarDialog* dlg)
{
    LOCKIT(_dialogsLock);
    DialogEntry* e = _head->find(dlg);

    if (!e)
        return;

    e->lock().unlock();
}

FarDialog* RunningDialogs::getDialog(HANDLE handle)
{
    LOCKIT(_dialogsLock);

    DialogEntry* res = _head->find(handle);
    if (!res)
    {
        return 0;
    }
    return res->dialog();
}

void RunningDialogs::unregisterDialog(FarDialog* dlg)
{
    LOCKIT(_dialogsLock);

    if (_head)
    {

        DialogEntry* p(0);
        DialogEntry* e = _head->find(dlg, &p);

        if (e)
        {
            e->deleteIt(p);

            if (e == _head)
            {
                _head = p;
            }
        }
    }

    TRACE("RunningDialogs::unregisterDialog called for %p\n", dlg);
}

intptr_t RunningDialogs::sendSafeMessage(HANDLE handle,
        intptr_t msg, intptr_t param0, void* param1)
{
    LOCKIT(_dialogsLock);

    FarDialog* d = getDialog(handle);
    if (!d)
        return 0;

    return SendDlgMessage(d->hwnd(), msg, param0, param1);
}

void RunningDialogs::postMessage(FarDialog* dlg,
        intptr_t msg, intptr_t param0, void* param1)
{
    LOCKIT(_dialogsLock);

    DialogEntry* e = _head->find(dlg);
    if (!e)
    {
        return;
    }

    bool doPost = true;
    switch (msg)
    {
    case DM_SETTEXTPTR:
        doPost = !e->setControlText(param0, (const wchar_t*)param1) && dlg->hwnd();
        break;
    default:
        e->addMessage(dlg->hwnd(), msg, param0, param1);
        break;
    }

    if (doPost)
    {
        MainThread* mainThread = MainThread::instance();
        if (mainThread)
        {
            mainThread->postDlgMessage();
        }
    }
}

intptr_t RunningDialogs::sendMessage(FarDialog* dlg, intptr_t msg, intptr_t param0, void* param1)
{
    Message m;
    {
        LOCKIT(_dialogsLock);
        if (!_activeDialog || (dlg != _activeDialog->dialog()))
            return 0;
        if (msg == DM_CLOSE)
        {
            DialogEntry* e = _head->find(dlg);
            if (e)
                e->messages().clear();
        }

        m.h = dlg->hwnd();
        m.message = msg;
        m.param1 = param0;
        m.param2 = param1;
    }
    MainThread* mainThread = MainThread::instance();

    if (!mainThread)
    {
        return 0;
    }

    return mainThread->sendDlgMessage(&m);
}

void RunningDialogs::processPostedDlgMessages(FarDialog* dlg)
{
    LOCKIT(_dialogsLock);

    if ((!_activeDialog) || (_activeDialog->dialog() != dlg))
        return;

    DialogEntry* e = _head->find(dlg);
    if (!e || !dlg->hwnd())
        return;

    size_t i;
    SendDlgMessage(dlg->hwnd(), DM_ENABLEREDRAW, FALSE, 0);
    for (i = 0; i < dlg->itemsCount(); i++)
    {
        MyStringW s;

        if (e->getControlText(i, s))
        {
            wchar_t* str = s;
            SendDlgMessage(dlg->hwnd(), DM_SETTEXTPTR, i, str);
        }
    }
    SendDlgMessage(dlg->hwnd(), DM_ENABLEREDRAW, TRUE, 0);

    for (i = 0; i < (int)e->messages().size(); i++)
    {
        Message& m = e->messages()[i];

        if (m.message != DM_SETTEXTPTR)
            SendDlgMessage(m.h, m.message, m.param1, m.param2);
    }
}

intptr_t RunningDialogs::processPostedSetText(HANDLE dlg,
        intptr_t id, const wchar_t* s)
{
    LOCKIT(_dialogsLock);

    DialogEntry* e = _head->find(dlg);

    if (!e)
    {
        return sendSafeMessage(dlg, DM_SETTEXTPTR, id, const_cast<wchar_t*>(s));
    }
    else
    {
        MyStringW str;
        if (e->getControlText(id, str))
        {
            s = str;
        }
        return sendSafeMessage(dlg, DM_SETTEXTPTR, id, const_cast<wchar_t*>(s));
    }
}

intptr_t RunningDialogs::processPostedMessage(HANDLE dlg,
        intptr_t msg, intptr_t param0, void* param1)
{
    LOCKIT(_dialogsLock);

    switch (msg)
    {
    case DM_SETTEXTPTR:
        return processPostedSetText(dlg, param0, (const wchar_t*)param1);
        break;
    default:
        return sendSafeMessage(dlg, msg, param0, param1);
    }
}

void RunningDialogs::notifyDialog(FarDialog* dlg, bool shown)
{
    LOCKIT(_dialogsLock);

    if (shown)
    {
        _activeDialog = _activeDialog->push(dlg);
        processPostedDlgMessages(dlg);
    }
    else
    {
        ASSERT(_activeDialog && _activeDialog->dialog() == dlg);
        processPostedDlgMessages(dlg);

        _activeDialog = _activeDialog->pop();
        if (_activeDialog)
        {
            processPostedDlgMessages(_activeDialog->dialog());
        }
    }
}

LONG_PTR RunningDialogs::processMessages(Message* msg)
{
    LOCKIT(_dialogsLock);

    if (msg)
    {
        if (!_activeDialog || _activeDialog->dialog()->hwnd() != msg->h)
            return 0;

        return SendDlgMessage(msg->h, msg->message, msg->param1, msg->param2);
    }
    else
    {
        if (_activeDialog)
        {
            processPostedDlgMessages(_activeDialog->dialog());
        }
    }
    return -1;
}

// vim: set et ts=4 ai :

