
/**
 * @file fardlg.cpp
 * Contains implementation of FarDialog class.
 *
 * $Id$
 */

#include "fardlg.h"
#include "mainthrd.h"
#include "dlgfmwk.h"
#include "mystring.h"

long WINAPI DefDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2);

FarDialog::FarDialog(): _hwnd(0), _running(false)
{
    RunningDialogs::instance()->registerDialog(this);
}

FarDialog::~FarDialog()
{
    hide();
    RunningDialogs::instance()->unregisterDialog(this);

    TRACE("Leaving FarDialog::~FarDialog\n");
}

int FarDialog::itemsCount()
{
    return 0;
}

InitDialogItem* FarDialog::items()
{
    return 0;
}

long FarDialog::dlgProc(HANDLE dlg, int msg, int param1, long param2)
{
    FarDialog* This;
    if (msg == DN_INITDIALOG)
    {
        This = reinterpret_cast<FarDialog*>(param2);
        if (This)
        {
            This->_hwnd = dlg;
            This->sendMessage(DM_SETDLGDATA, 0, (long)This);
            RunningDialogs::instance()->notifyDialog(This, true);
        }
    }
    else
    {
        This = (FarDialog*)RunningDialogs::instance()->sendSafeMessage(dlg, DM_GETDLGDATA, 0, 0);
        if (This && !RunningDialogs::instance()->lockDialog(This))
            This = 0;
    }

    if (!This)
        return DefDlgProc(dlg, msg, param1, param2);

    long res = This->handle(msg, param1, param2);

    if (msg == DN_CLOSE && res)
    {
        This->restoreItems();
        This->_hwnd = 0;
        RunningDialogs::instance()->notifyDialog(This, false);
    }
    RunningDialogs::instance()->unlockDialog(This);

    return res;
}

bool FarDialog::onInit()
{
    return false;
}

bool FarDialog::onClose(int /*closeId*/)
{
    /** @todo After the dialog has been shown copy flags and other stuff to the static items */
    return true;
}

long FarDialog::handle(int msg, int param1, long param2)
{
    switch (msg)
    {
    case DN_INITDIALOG:
        return (long)onInit();
    case DN_CLOSE:
        {
            long res = (long)onClose(param1);
            _running = false;
            return res;
        }
        break;
    default:
        return DefDlgProc(hwnd(), msg, param1, param2);
    }
}

/**
 * Hides running dialog
 */
int FarDialog::hide()
{
    int res = sendMessage(DM_CLOSE, 0, 0);

    _hwnd = 0;
    
    return res;
}

int FarDialog::doShow()
{
    if (!RunningDialogs::instance()->lockDialog(this))
    {
        return -1;
    }

    void* farItems;
    int res = run(farItems);

    /** @todo After the dialog has been shown copy flags and other stuff to the static items */
    if (RunningDialogs::instance()->lockDialog(this))
    {
        restoreItems();
        RunningDialogs::instance()->unlockDialog(this);
    }
    freeItems(farItems);
                                                  
    return res;
}

/**
 * @brief Dialog shower
 *
 * Implementation of MainThread::Callable interface used to
 * inter-thread communications during dialog showing
 */
class DialogShower : public MainThread::Callable
{
private:
    FarDialog* _dlg;
public:
    DialogShower(FarDialog* dlg):_dlg(dlg){}
    void* call()
    {
        if (_dlg)
            return (void*)_dlg->doShow();

        return (void*)-1;
    }
};

int FarDialog::show(bool modal)
{
    _running = true;
    DialogShower* d = new DialogShower(this);
    if (modal)
        return reinterpret_cast<int>(MainThread::instance()->callIt(d));

    MainThread::instance()->callItAsync(d);
    return -1;
}

int FarDialog::left()
{
    return -1;
}

int FarDialog::top()
{
    return -1;
}

int FarDialog::right()
{
    InitDialogItem* pitems = items();
    if (pitems)
        return pitems->X2 + 4;
    return 0;
}

int FarDialog::bottom()
{
    InitDialogItem* pitems = items();
    if (pitems)
        return pitems->Y2+2;
    return 0;
}

const wchar_t* FarDialog::help()
{
    return 0;
}

DWORD FarDialog::flags()
{
    return 0;
}

long FarDialog::sendMessage(int msg, int param1, long param2)
{
    return RunningDialogs::instance()->sendMessage(this, msg, param1, param2);
}

void FarDialog::postMessage(int msg, int param1, long param2)
{
    RunningDialogs::instance()->postMessage(this, msg, param1, param2);
}

/**
 * @return - previous state of the control
 */
bool FarDialog::enable(int id)
{
    if (running())
    {
        return sendMessage(DM_ENABLE, id, TRUE)?true:false;
    }
    else
    {
        bool ret = !((items()[id].Flags & DIF_DISABLE) == DIF_DISABLE);
        items()[id].Flags &= ~DIF_DISABLE;

        return ret;
    }
}

/**
 * @return - previous state of the control
 */
bool FarDialog::disable(int id)
{
    if (running())
    {
        return sendMessage(DM_ENABLE, id, FALSE)?true:false;
    }
    else
    {
        bool ret = ((items()[id].Flags & DIF_DISABLE) == DIF_DISABLE);
        items()[id].Flags |= DIF_DISABLE;

        return ret;
    }
}

int FarDialog::switchCheckBox(int id, int state)
{
    if (running())
    {
        return sendMessage(DM_SETCHECK, id, state);
    }
    else
    {
        int ret = (items()[id].Selected & 0x3);
        items()[id].Selected |= state;

        return ret;
    }
}

int FarDialog::checkState(int id)
{
    if (running())
    {
        return sendMessage(DM_GETCHECK, id, 0);
    }
    else
    {
        return items()[id].Selected;
    }
}

bool FarDialog::lock()
{
    return RunningDialogs::instance()->lockDialog(this);
}

void FarDialog::unlock()
{
    RunningDialogs::instance()->unlockDialog(this);
}

// vim: set et ts=4 ai :

