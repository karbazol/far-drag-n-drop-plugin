/**
 * @file plugin/ansi/fardlg_a.cpp
 *
 * $Id$
 */

#include "../fardlg.h"

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
    if (!PInit || !PItem)
    {
        return;
    }

    for (I=0; I < ItemsNumber; I++,PItem++,PInit++)
    {
        memset(PItem, 0, sizeof(*PItem));
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

void FarDialog::restoreItems()
{
    InitDialogItem* initItems = items();
    if (initItems)
    {
        for (int i = 0; i < itemsCount(); i++)
        {
            FarDialogItem item = {0};
            if (sendMessage(DM_GETDLGITEM, i, reinterpret_cast<LONG_PTR>(&item)))
            {
                initItems[i].Selected = item.Selected;
            }
        }
    }
}

void FarDialog::freeFarItems(void* farItems)
{
    delete [] reinterpret_cast<FarDialogItem*>(farItems);
}

// vim: set et ts=4 sw=4 ai :

