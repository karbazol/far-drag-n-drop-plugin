// $Id: dlgfmwk.cpp 26 2008-04-20 18:48:32Z eleskine $

#include "dlgfmwk.h"
#include "dll.h"
#include "mystring.h"
#include "growarry.h"
#include "mainthrd.h"

class DialogEntry
{
private:
    struct ControlTextMessage
    {
        bool queued;
        MyStringA text;
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

    DialogEntry* _next;
    FarDialog* _dlg;
    CriticalSection _lock;
    ControlTexts _texts;
    DialogEntry():_next(0),_dlg(0),_lock(),_texts()
    {
    }
    DialogEntry(DialogEntry* head, FarDialog* dlg):
        _next(head), _dlg(dlg),_lock(),_texts()
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
    ~DialogEntry(){}
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

    bool setControlText(int id, const char* s)
    {
        if (!_texts.size())
            _texts.size(_dlg->itemsCount());
        ControlTextMessage& m = _texts[id];
        bool res = m.queued;
        m.text = s;
        m.queued = true;

        return res;
    }

    bool getControlText(int id, MyStringA& s)
    {
        if (!_texts.size())
            _texts.size(_dlg->itemsCount());
        ControlTextMessage& m = _texts[id];
        bool res = m.queued;
        m.queued = false;

        s = m.text;

        return res;
    }
};

RunningDialogs::RunningDialogs():_head(0), _dialogsLock()
{
}

RunningDialogs::~RunningDialogs()
{
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

        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
    }

    return p;
}

void RunningDialogs::registerDialog(FarDialog* dlg)
{
    lockIt l(&_dialogsLock);
    _head = _head->newEntry(dlg);
}

bool RunningDialogs::lockDialog(FarDialog* dlg)
{
    lockIt l(&_dialogsLock);
    DialogEntry* e = _head->find(dlg);
    if (!e)
        return false;

    e->lock().enter();

    return true;
}

void RunningDialogs::unlockDialog(FarDialog* dlg)
{
    lockIt l(&_dialogsLock);
    DialogEntry* e = _head->find(dlg);

    if (!e)
        return;

    e->lock().leave();
}

FarDialog* RunningDialogs::getDialog(void* handle)
{
    lockIt l(&_dialogsLock);

    DialogEntry* res = _head->find(reinterpret_cast<HANDLE>(handle));
    if (!res)
    {
        return 0;
    }
    return res->dialog();
}

void RunningDialogs::unregisterDialog(FarDialog* dlg)
{
    lockIt l(&_dialogsLock);

    DialogEntry* p;
    DialogEntry* e = _head->find(dlg, &p);

    e->deleteIt(p);

    if (e == _head)
        _head = p;
}

long RunningDialogs::sendSafeMessage(HANDLE handle,
        int msg, int param0, long param1)
{
    FarDialog* d = getDialog(handle);
    if (!d)
        return 0;

    return SendDlgMessage(d->hwnd(), msg, param0, param1);
}

void RunningDialogs::postMessage(FarDialog* dlg,
        int msg, int param0, long param1)
{
    lockIt l(&_dialogsLock);

    DialogEntry* e = _head->find(dlg);
    if (!e)
        return;

    switch (msg)
    {
    case DM_SETTEXTPTR:
        if (!e->setControlText(param0, (const char*)param1) && dlg->hwnd())
            MainThread::instance()->postDlgMessage(dlg->hwnd(), msg, param0, param1);
        break;
    default:
        MainThread::instance()->postDlgMessage(dlg->hwnd(), msg, param0, param1);
    }
}

void RunningDialogs::postMyMessages(FarDialog* dlg)
{
    lockIt l(&_dialogsLock);

    DialogEntry* e = _head->find(dlg);
    if (!e || !dlg->hwnd())
        return;

    int i;
    for (i = 0; i < dlg->itemsCount(); i++)
    {
        MyStringA s;

        if (e->getControlText(i, s))
        {
            char* str = s;
            e->setControlText(i, s);
            MainThread::instance()->postDlgMessage(dlg->hwnd(), DM_SETTEXTPTR, i, (long)str);
        }
    }
}

long RunningDialogs::processPostedSetText(HANDLE dlg,
        int id, const char* s)
{
    lockIt l(&_dialogsLock);

    DialogEntry* e = _head->find(dlg);

    if (!e)
        return sendSafeMessage(dlg, DM_SETTEXTPTR, id, (long)s);
    else
    {
        MyStringA str;
        if (e->getControlText(id, str))
            s = str;
        return sendSafeMessage(dlg, DM_SETTEXTPTR, id, (long)s);
    }
}

long RunningDialogs::processPostedMessage(HANDLE dlg,
        int msg, int param0, long param1)
{
    switch (msg)
    {
    case DM_SETTEXTPTR:
        return processPostedSetText(dlg, param0, (const char*)param1);
        break;
    default:
        return sendSafeMessage(dlg, msg, param0, param1);
    }
}

// vim: set et ts=4 ai :

