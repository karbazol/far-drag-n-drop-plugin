/**
 * @file plugin/unicode/fardlg_u.cpp
 *
 * $Id$
 */

#include "fardlg.h"

HANDLE DialogInit(int X1, int Y1, int X2, int Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, long Param=NULL);
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

int FarDialog::run(void*& farItems)
{
    int count = itemsCount();
    FarDialogItem* theItems = new FarDialogItem[count];
    farItems = theItems;
    InitDialogItems(items(), theItems, count);

    _hwnd = DialogInit(left(), top(), right(), bottom(), help(), theItems, count, 0,
            flags(), &dlgProc, (long)this);

    return DialogRun(_hwnd);
}

void FarDialog::restoreItems(void* farItems)
{
    InitDialogItem* initItems = items();
    if (initItems)
    {
        RestoreDialogItems(reinterpret_cast<FarDialogItem*>(farItems), initItems, itemsCount());
    }
}

void FarDialog::freeItems(void* farItems)
{
    delete [] reinterpret_cast<FarDialogItem*>(farItems);
}

// vim: set et ts=4 sw=4 ai :

