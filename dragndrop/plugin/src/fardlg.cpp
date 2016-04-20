
/**
 * @file fardlg.cpp
 * Contains implementation of FarDialog class.
 *
 * $Id: fardlg.cpp 81 2011-11-07 08:50:02Z Karbazol $
 */

#include <dll/dll_utils.h>
#include <dll/mystring.h>

#include "fardlg.h"
#include "mainthrd.h"
#include "dlgfmwk.h"

LONG_PTR WINAPI DefDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2);

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

size_t FarDialog::itemsCount()
{
    return 0;
}

InitDialogItem* FarDialog::items()
{
    return 0;
}

intptr_t FarDialog::dlgProc(HANDLE dlg, intptr_t msg, intptr_t param1, void* param2)
{
    FarDialog* This = 0;
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (msg == DN_INITDIALOG)
    {
        This = reinterpret_cast<FarDialog*>(param2);
        if (This)
        {
            InterlockedExchangePointer(&This->_hwnd, dlg);
            This->sendMessage(DM_SETDLGDATA, 0, This);
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
    intptr_t res = This->handle(msg, param1, param2);

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

intptr_t FarDialog::handle(intptr_t msg, intptr_t param1, void* param2)
{
    switch (msg)
    {
    case DN_INITDIALOG:
        return onInit();
    case DN_CLOSE:
        {
            intptr_t res = onClose(param1);
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
intptr_t FarDialog::hide()
{
    intptr_t res = sendMessage(DM_CLOSE, 0, 0);

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

intptr_t FarDialog::sendMessage(intptr_t msg, intptr_t param1, void* param2)
{
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (runningDialogs)
    {
        return runningDialogs->sendMessage(this, msg, param1, param2);
    }

    return 0;
}

void FarDialog::postMessage(intptr_t msg, intptr_t param1, void* param2)
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
        return sendMessage(DM_ENABLE, id, (void*)TRUE)?true:false;
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
        intptr_t longstate = state;
        return static_cast<int>(sendMessage(DM_SETCHECK, id, reinterpret_cast<void*>(longstate)));
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

HANDLE DialogInit(const GUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, size_t ItemsNumber,
  intptr_t Reserved, FARDIALOGFLAGS Flags, FARWINDOWPROC DlgProc, void* Param=NULL);
intptr_t DialogRun(HANDLE hdlg);
void DialogFree(HANDLE hDlg);

/**
 * This function converts a vector of InitDialogItem objects to vector of FarDialogItem objects
 * used by Dialog and DialogEx functions.
 */
static void InitDialogItems(
       const struct InitDialogItem *Init,
       struct FarDialogItem *Item,
       size_t ItemsNumber
)
{
    size_t I;      
    const struct InitDialogItem *PInit=Init;
    struct FarDialogItem *PItem=Item;
    for (I=0; I < ItemsNumber; I++,PItem++,PInit++)
    {
        PItem->Type=PInit->Type;
        PItem->X1=PInit->X1;
        PItem->Y1=PInit->Y1;
        PItem->X2=PInit->X2;
        PItem->Y2=PInit->Y2;
        PItem->Selected=PInit->Selected;
        PItem->Flags=PInit->Flags;
        if ((uintptr_t)PInit->Data < 2000)
        {
           PItem->Data = GetMsg((int)(uintptr_t)PInit->Data);
        }
        else
        {
            PItem->Data = PInit->Data;
        }
        
        PItem->MaxLength = lstrlen(PItem->Data);
    }
}

intptr_t FarDialog::run(void*& farItems)
{
    size_t count = itemsCount();
    FarDialogItem* theItems = new FarDialogItem[count];
    farItems = theItems;
    InitDialogItems(items(), theItems, count);

    _hwnd = DialogInit(&Id(), left(), top(), right(), bottom(), help(), theItems, count, 0,
            flags(), &dlgProc, this);

    return DialogRun(_hwnd);
}

void FarDialog::restoreItems()
{
    InitDialogItem* initItems = items();
    FarDialogItem* item;
    if (initItems)
    {
        size_t i;
        for (i = 0; i < itemsCount(); i++)
        {
            item = reinterpret_cast<FarDialogItem*>(malloc(sendMessage(DM_GETDLGITEM, i, 0)));
            if (item)
            {
                sendMessage(DM_GETDLGITEM, i, item);
                //initItems[i].Selected = item->Selected;
                free(item);
            }
        }
    }
}

void FarDialog::freeFarItems(void* farItems)
{
    DialogFree(_hwnd);
    _hwnd = 0;
    delete [] reinterpret_cast<FarDialogItem*>(farItems);
}

bool FarDialog::checked(int id)
{
    return checkState(id) != BSTATE_UNCHECKED;
}

// vim: set et ts=4 ai :

