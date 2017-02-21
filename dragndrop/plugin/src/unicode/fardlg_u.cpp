/**
 * @file plugin/unicode/fardlg_u.cpp
 *
 * $Id$
 */

#include "../fardlg.h"

HANDLE DialogInit(const GUID* guid, int X1, int Y1, int X2, int Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, FAR_LPARAM_TYPE Param=NULL);
intptr_t DialogRun(HANDLE hdlg);
void DialogFree(HANDLE hDlg);

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
#ifdef FAR2
        PItem->Type=PInit->Type;
        PItem->X1=PInit->X1;
        PItem->Y1=PInit->Y1;
        PItem->X2=PInit->X2;
        PItem->Y2=PInit->Y2;
        PItem->Focus=PInit->Focus;
        PItem->Selected=PInit->Selected;
        PItem->Flags=PInit->Flags;
        PItem->DefaultButton=PInit->DefaultButton;
        if ((uintptr_t)PInit->Data < 2000)
        {
           PItem->PtrData = GetMsg((int)(uintptr_t)PInit->Data);
        }
        else
        {
            PItem->PtrData = PInit->Data;
        }
        
        PItem->MaxLen = lstrlen(PItem->PtrData);
#else
        PItem->Type = PInit->Type;
        PItem->X1 = PInit->X1;
        PItem->Y1 = PInit->Y1;
        PItem->X2 = PInit->X2;
        PItem->Y2 = PInit->Y2;
        PItem->Selected = PInit->Selected;
        PItem->History = nullptr;
        PItem->Mask = nullptr;
        PItem->Flags = PInit->Flags;
        if ((uintptr_t)PInit->Data < 2000)
            PItem->Data = GetMsg((int)(uintptr_t)PInit->Data);
        else
            PItem->Data = PInit->Data;
        PItem->MaxLength = lstrlen(PItem->Data);
        PItem->UserData = 0;
        PItem->Reserved[0] = 0;
        PItem->Reserved[1] = 0;
#endif
    }
}

intptr_t FarDialog::run(void*& farItems)
{
    int count = itemsCount();
    FarDialogItem* theItems = new FarDialogItem[count];
    farItems = theItems;
    InitDialogItems(items(), theItems, count);

    _hwnd = DialogInit(guid(), left(), top(), right(), bottom(), help(), theItems, count, 0,
            flags(), &dlgProc, (FAR_LPARAM_TYPE)this);

    return DialogRun(_hwnd);
}

void FarDialog::restoreItems()
{
    InitDialogItem* initItems = items();
    FarDialogItem* item;
    if (initItems)
    {
        int i;
        for (i = 0; i < itemsCount(); i++)
        {
            size_t size = sendMessage(DM_GETDLGITEM, i, 0);
            item = reinterpret_cast<FarDialogItem*>(malloc(size));
            if (item)
            {
#ifdef FAR2
                sendMessage(DM_GETDLGITEM, i, reinterpret_cast<FAR_LPARAM_TYPE>(item));
#else
                FarGetDialogItem gdi = { sizeof(gdi), size, item };
                sendMessage(DM_GETDLGITEM, i, &gdi);
#endif
                initItems[i].Selected = (item->Selected != 0);
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

// vim: set et ts=4 sw=4 ai :
