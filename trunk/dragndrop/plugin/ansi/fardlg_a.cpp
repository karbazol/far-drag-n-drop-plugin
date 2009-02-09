/**
 * @file plugin/ansi/fardlg_a.cpp
 *
 * $Id$
 */

#include "fardlg.h"

int DialogEx(int X1, int Y1, int X2, int Y2,
  const char *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, long Param=NULL);

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
            WideCharToMultiByte(CP_OEMCP, 0, GetMsg((int)PInit->Data), -1, PItem->Data, 
                    LENGTH(PItem->Data), 0, 0);
        else
            WideCharToMultiByte(CP_OEMCP, 0, PInit->Data, -1, PItem->Data, 
                    LENGTH(PItem->Data), 0, 0);
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

    MyStringA helpTopic = w2a(help(), CP_OEMCP);

    return DialogEx(left(), top(), right(), bottom(), helpTopic, theItems, count, 0,
                flags(), &dlgProc, (long)this);
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

