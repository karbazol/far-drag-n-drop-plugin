
/**
 * @file fardlg.cpp
 * Contains implementation of FarDialog class.
 *
 * $Id: fardlg.cpp 88 2009-01-09 15:20:59Z eleskine $
 */

#include "fardlg.h"
#include "mainthrd.h"
#include "dlgfmwk.h"
#include "mystring.h"

long WINAPI DefDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2);

HANDLE DialogInit(int X1, int Y1, int X2, int Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc=&DefDlgProc, long Param=NULL);
int DialogRun(HANDLE hdlg);

/**
 * This function converts a vector of InitDialogItem objects to vector of FarDialogItem objects
 * used by Dialog and DialogEx functions.
 */
static void InitDialogItems(
       const struct InitDialogItem *Init,
       struct FarDialogItem *Item,
       int ItemsNumber
)
{
    int I;      
    const struct InitDialogItem *PInit=Init;
    struct FarDialogItem *PItem=Item;
    for (I=0; I < ItemsNumber; I++,PItem++,PInit++)
    {
        PItem->Type=PInit->Type;
        PItem->X1=PInit->X1;
        PItem->Y1=PInit->Y1;
        PItem->X2=PInit->X2;
        PItem->Y2=PInit->Y2;
        PItem->Focus=PInit->Focus;
        PItem->Selected=PInit->Selected;
        PItem->Flags=PInit->Flags;
        PItem->DefaultButton=PInit->DefaultButton;
        if ((unsigned int)PInit->Data < 2000)
        {
           PItem->PtrData = GetMsg((int)PInit->Data);
        }
        else
        {
            PItem->PtrData = PInit->Data;
        }
        
        PItem->MaxLen = lstrlen(PItem->PtrData);
    }
}

static void RestoreDialogItems(
        const struct FarDialogItem *Item,
        struct InitDialogItem *Init,
        int ItemsNumber
)
{
    int i;
    for (i = 0; i < ItemsNumber; i++, Item++, Init++)
    {
        Init->Type = Item->Type;
        Init->X1 = Item->X1;
        Init->Y1 = Item->Y1;
        Init->X2 = Item->X2;
        Init->Y2 = Item->Y2;
        Init->Focus = Item->Focus;
        Init->Selected = Item->Selected;
        Init->Flags = Item->Flags;
        Init->DefaultButton = Item->DefaultButton;

        /** @todo Deal with string data */
    }
}

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
            _hwnd = 0;
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
        _running = false;
        return -1;
    }

    int count = itemsCount();
    FarDialogItem* theItems = new FarDialogItem[count];
    InitDialogItems(items(), theItems, count);

    _hwnd = DialogInit(left(), top(), right(), bottom(), help(), theItems, count, 0,
            flags(), &dlgProc, (long)this);

    int res = DialogRun(_hwnd);

    /** @todo After the dialog has been shown copy flags and other stuff to the static items */
    if (RunningDialogs::instance()->lockDialog(this))
    {
        InitDialogItem* initItems = items();
        if (initItems)
            RestoreDialogItems(theItems, initItems, count);
        RunningDialogs::instance()->unlockDialog(this);
    }
    delete [] theItems;
                                                  
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

