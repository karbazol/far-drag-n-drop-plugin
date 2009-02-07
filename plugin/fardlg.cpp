#include "fardlg.h"
#include "mainthrd.h"
#include "dlgfmwk.h"
#include "mystring.h"

FarDialog::FarDialog(): _hwnd(0), _running(false)
{
    RunningDialogs::instance()->registerDialog(this);
}

FarDialog::~FarDialog()
{
    hide();
    RunningDialogs::instance()->unregisterDialog(this);
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
            RunningDialogs::instance()->postMyMessages(This);
        }
    }
    else
    {
        This = (FarDialog*)MainThread::instance()->sendDlgMessage(dlg, DM_GETDLGDATA, 0, 0);
        if (This)
            RunningDialogs::instance()->lockDialog(This);
    }

    if (!This)
        return DefDlgProc(dlg, msg, param1, param2);

    long res = This->handle(msg, param1, param2);
    RunningDialogs::instance()->unlockDialog(This);

    return res;
}

bool FarDialog::onInit()
{
    return false;
}

bool FarDialog::onClose(int /*closeId*/)
{
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

    int res = DialogEx(left(), top(), right(), bottom(), help(), theItems, count, 0,
                flags(), &dlgProc, (long)this);

    delete [] theItems;

    return res;
}

class DialogShower : public MainThread::Callable
{
private:
    FarDialog* _dlg;
public:
    DialogShower(FarDialog* dlg):_dlg(dlg){}
    void* call()
    {
        if (_dlg)
            return (void*)_dlg->show(true);

        return (void*)-1;
    }
};

int FarDialog::show(bool modal)
{
    _running = true;
    if (modal)
        return doShow();

    DialogShower* d = new DialogShower(this);
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

    return 0;
}

int FarDialog::bottom()
{
    return 0;
}

const char* FarDialog::help()
{
    return 0;
}

DWORD FarDialog::flags()
{
    return 0;
}

long FarDialog::sendMessage(int msg, int param1, long param2)
{
    return MainThread::instance()->sendDlgMessage(hwnd(), msg, param1, param2);
}

void FarDialog::postMessage(int msg, int param1, long param2)
{
    RunningDialogs::instance()->postMessage(this, msg, param1, param2);
}

// vim: set et ts=4 ai :

