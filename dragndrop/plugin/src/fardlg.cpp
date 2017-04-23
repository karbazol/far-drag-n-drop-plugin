/**
 * @file fardlg.cpp
 * Contains implementation of FarDialog class.
 *
 */

#include <refcounted.hpp>
#include <dll_utils.h>
#include <mystring.h>

#include "fardlg.h"
#include "mainthrd.h"

LONG_PTR WINAPI DefDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2);

FarDialog::FarDialog(): RefCounted(), _running(false), _hwnd(0), _textGuard(), _controlTexts(0)
{
}

FarDialog::~FarDialog()
{
    hide();
    if (_controlTexts)
    {
        delete [] _controlTexts;
        _controlTexts = nullptr;
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
    if (msg == DN_INITDIALOG)
    {
        This = reinterpret_cast<FarDialog*>(param2);
        if (This)
        {
            InterlockedExchangePointer(&This->_hwnd, dlg);
            This->sendMessage(DM_SETDLGDATA, 0, This);
        }
    }
    else
    {
        This = reinterpret_cast<FarDialog*>(SendDlgMessage(dlg, DM_GETDLGDATA, 0, 0));
    }

    if (!This)
    {
        return DefDlgProc(dlg, msg, param1, param2);
    }

    intptr_t res = This->handle(msg, param1, param2);

    if (msg == DN_CLOSE && res)
    {
        This->restoreItems();
        InterlockedExchangePointer(&This->_hwnd, 0);
    }

    return res;
}

intptr_t FarDialog::setText(intptr_t id, const wchar_t* value)
{
    if (!value || !running())
    {
        return 0;
    }

    MainThread* mainThread = MainThread::instance();
    if (!mainThread)
    {
        return 0;
    }

    if (mainThread->isMainThread())
    {
        return sendMessage(DM_SETTEXTPTR, id, const_cast<wchar_t*>(value)); 
    }
    else
    {
        LOCKIT(_textGuard);


        if (!_controlTexts)
        {
            intptr_t count = itemsCount();
            _controlTexts = new MyStringW*[count];
            memset(_controlTexts, 0, sizeof(*_controlTexts)*count);
            addRef();
            mainThread->callItAsync([](void* that)
                    {
                        FarDialog* This = reinterpret_cast<FarDialog*>(that);
                        LOCKIT(This->_textGuard);
                        intptr_t count = static_cast<intptr_t>(This->itemsCount());
                        MyStringW** p = This->_controlTexts;
                        This->_controlTexts = nullptr;
                        for (intptr_t i = 0; i < count; ++i)
                        {
                            if (p[i])
                            {
                                This->sendMessage(DM_SETTEXTPTR, i, static_cast<wchar_t*>(*p[i]));
                                delete p[i];
                                p[i] = nullptr;
                            }
                        }
                        delete [] p;
                        This->release();
                        return static_cast<void*>(nullptr);
                    }, this);
        }
        if (_controlTexts[id])
        {
            *_controlTexts[id] = value;
        }
        else
        {
            _controlTexts[id] = new MyStringW(value);
        }
        return lstrlen(value);
    }
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

    CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {0};
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);
    prepareItems(consoleInfo.dwSize.X, consoleInfo.dwSize.Y);

    void* farItems;
    res = run(farItems);

//    restoreItems();
    freeFarItems(farItems);
    releaseItems();

    return res;
}

intptr_t FarDialog::show(bool modal)
{
    MainThread* mainThread = MainThread::instance();
    if (!mainThread)
    {
        return -1;
    }

    _running = true;
    if (modal)
    {
        intptr_t result = reinterpret_cast<intptr_t>(mainThread->callIt([](void* param)
                    {
                        return reinterpret_cast<void*>(reinterpret_cast<FarDialog*>(param)->doShow());
                    }, this));
        _running = false;
        return result;
    }

    addRef();
    mainThread->callItAsync([](void* param)
                {
                    FarDialog* that = reinterpret_cast<FarDialog*>(param);
                    that->doShow();
                    that->_running = false;
                    that->release();
                    return reinterpret_cast<void*>(-1);
                }, this);
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
    MainThread* mainThread = MainThread::instance();
    if (!mainThread)
    {
        return 0;
    }

    if (mainThread->isMainThread())
    {
        if (!_hwnd)
        {
            return 0;
        }
        return SendDlgMessage(_hwnd, msg, param1, param2);
    }
    else
    {
        struct Params {
            FarDialog* This;
            intptr_t msg;
            intptr_t param1;
            void* param2;
        } params = {this, msg, param1, param2};
        return reinterpret_cast<intptr_t>(mainThread->callIt([](void* param) {
                Params* params = static_cast<Params*>(param);
                return reinterpret_cast<void*>(params->This->sendMessage(
                            params->msg, params->param1, params->param2));
                }, &params));
    }
    return 0;
}

/**
 * @return - previous state of the control
 */
bool FarDialog::enable(intptr_t id)
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
bool FarDialog::disable(intptr_t id)
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

intptr_t FarDialog::switchCheckBox(intptr_t id, intptr_t state)
{
    if (running())
    {
        return sendMessage(DM_SETCHECK, id, reinterpret_cast<void*>(state));
    }
    else
    {
        intptr_t ret = items()[id].Selected;
        items()[id].Selected = state;

        return ret;
    }
}

intptr_t FarDialog::checkState(intptr_t id)
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
        memset(PItem, 0, sizeof(*PItem));
        PItem->Type=PInit->Type;
        PItem->X1=PInit->X1;
        PItem->Y1=PInit->Y1;
        PItem->X2=PInit->X2;
        PItem->Y2=PInit->Y2;
        PItem->Selected=PInit->Selected;
        PItem->Flags=PInit->Flags;
        if (PInit->DefaultButton)
            PItem->Flags |= DIF_DEFAULTBUTTON;
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

    if (_hwnd == INVALID_HANDLE_VALUE)
    {
        TRACE("Could not initialize dialog");
        LASTERROR();
        return -2;
    }

    return DialogRun(_hwnd);
}

void FarDialog::restoreItems()
{
    InitDialogItem* initItems = items();
    if (initItems)
    {
        size_t i;
        for (i = 0; i < itemsCount(); i++)
        {
            FarGetDialogItem item = {
                sizeof(item),
                static_cast<size_t>(sendMessage(DM_GETDLGITEM, i, 0))
            };
            if (static_cast<intptr_t>(item.Size) > 0)
            {
                item.Item = reinterpret_cast<FarDialogItem*>(malloc(item.Size));
                if (item.Item)
                {
                    sendMessage(DM_GETDLGITEM, i, &item);
                    initItems[i].Selected = item.Item->Selected;
                    free(item.Item);
                }
            }
        }
    }
}

void FarDialog::freeFarItems(void* farItems)
{
    if (_hwnd)
    {
        DialogFree(_hwnd);
        _hwnd = 0;
    }
    delete [] reinterpret_cast<FarDialogItem*>(farItems);
}

bool FarDialog::checked(intptr_t id)
{
    return checkState(id) != BSTATE_UNCHECKED;
}

// vim: set et ts=4 ai :

