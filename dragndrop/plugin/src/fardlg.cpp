
/**
 * @file fardlg.cpp
 * Contains implementation of FarDialog class.
 *
 * $Id$
 */

#include "dll_utils.h"
#include "fardlg.h"
#include "mainthrd.h"
#include "dlgfmwk.h"
#include "mystring.h"

FAR_RETURN_TYPE WINAPI DefDlgProc(HANDLE hDlg, FAR_WPARAM_TYPE Msg, FAR_WPARAM_TYPE Param1, FAR_LPARAM_TYPE Param2);

FarDialog::FarDialog(): _hwnd(0), _running(0), _destructing(0)
{
    _running = CreateEvent(NULL, TRUE, TRUE, NULL);
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs)
    {
        runningDialogs->registerDialog(this);
    }
}

FarDialog::~FarDialog()
{
    InterlockedExchange(&_destructing, 1L);
    hide();
    WaitForSingleObject(_running, INFINITE);
    CloseHandle(_running);
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs)
    {
        runningDialogs->unregisterDialog(this);
    }

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

FAR_RETURN_TYPE FarDialog::dlgProc(HANDLE dlg, FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param1, FAR_LPARAM_TYPE param2)
{
    FarDialog* This = 0;
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (msg == DN_INITDIALOG)
    {
        This = reinterpret_cast<FarDialog*>(param2);
        if (This)
        {
            InterlockedExchangePointer(&This->_hwnd, dlg);
            This->sendMessage(DM_SETDLGDATA, 0, (FAR_LPARAM_TYPE)This);
            if (runningDialogs)
            {
                runningDialogs->notifyDialog(This, true);
            }
        }
    }
    else
    {
        if (runningDialogs)
        {
            This = (FarDialog*)runningDialogs->sendSafeMessage(dlg, DM_GETDLGDATA, 0, 0);
            if (This && !runningDialogs->lockDialog(This))
            {
                This = 0;
            }
        }
    }

    if (!This)
    {
        return DefDlgProc(dlg, msg, param1, param2);
    }

    if (This->_destructing)
    {
        if (msg != DN_CLOSE)
        {
            This->sendMessage(DM_CLOSE, 0, 0);
            return DefDlgProc(dlg, msg, param1, param2);
        }
        return TRUE;
    }
    FAR_RETURN_TYPE res = This->handle(msg, param1, param2);

    if (msg == DN_CLOSE && res)
    {
        This->restoreItems();
        //This->_hwnd = 0;
        if (runningDialogs)
        {
            runningDialogs->notifyDialog(This, false);
        }

        return res;
    }

    if (runningDialogs)
    {
        runningDialogs->unlockDialog(This);
    }

    return res;
}

bool FarDialog::onInit()
{
    return false;
}

bool FarDialog::onClose(intptr_t /*closeId*/)
{
    return true;
}

FAR_RETURN_TYPE FarDialog::handle(FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param1, FAR_LPARAM_TYPE param2)
{
    switch (msg)
    {
    case DN_INITDIALOG:
        return (LONG_PTR)onInit();
    case DN_CLOSE:
        {
            LONG_PTR res = (LONG_PTR)onClose(param1);
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
    int res = static_cast<int>(sendMessage(DM_CLOSE, 0, 0));

    if (res)
    {
        InterlockedExchangePointer(&_hwnd, 0);
    }

    return res;
}

intptr_t FarDialog::doShow()
{
    intptr_t res = -1;
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs && runningDialogs->lockDialog(this))
    {
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {0};
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);
        prepareItems(consoleInfo.dwSize.X, consoleInfo.dwSize.Y);

        void* farItems;
        res = run(farItems);

//        restoreItems();
        freeFarItems(farItems);
        releaseItems();

        runningDialogs->unlockDialog(this);
    }

    SetEvent(_running);

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

intptr_t FarDialog::show(bool modal)
{
    MainThread* mainThread = MainThread::instance();
    if (!mainThread)
    {
        return -1;
    }

    ResetEvent(_running);
    DialogShower* d = new DialogShower(this);
    if (modal)
    {
        return reinterpret_cast<intptr_t>(mainThread->callIt(d));
    }

    mainThread->callItAsync(d);
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
    {
        return pitems->X2 + 4;
    }
    return 0;
}

int FarDialog::bottom()
{
    InitDialogItem* pitems = items();
    if (pitems)
    {
        return pitems->Y2+2;
    }
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

FAR_RETURN_TYPE FarDialog::sendMessage(FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param1, FAR_LPARAM_TYPE param2)
{
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs)
    {
        return runningDialogs->sendMessage(this, msg, param1, param2);
    }

    return 0;
}

void FarDialog::postMessage(FAR_WPARAM_TYPE msg, FAR_WPARAM_TYPE param1, FAR_LPARAM_TYPE param2)
{
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs)
    {
        runningDialogs->postMessage(this, msg, param1, param2);
    }
}

/**
 * @return - previous state of the control
 */
bool FarDialog::enable(int id)
{
    if (running())
    {
        return sendMessage(DM_ENABLE, id, (FAR_LPARAM_TYPE)TRUE)?true:false;
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
        return (int)sendMessage(DM_SETCHECK, id, (FAR_LPARAM_TYPE)(intptr_t)state);
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
        return static_cast<int>(sendMessage(DM_GETCHECK, id, 0));
    }
    else
    {
        return items()[id].Selected;
    }
}

bool FarDialog::lock()
{
    RunningDialogs* runningDialogs = RunningDialogs::instance();

    if (!runningDialogs)
    {
        return false;
    }

    return runningDialogs->lockDialog(this);
}

void FarDialog::unlock()
{
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs)
    {
        runningDialogs->unlockDialog(this);
    }
}

// vim: set et ts=4 ai :

