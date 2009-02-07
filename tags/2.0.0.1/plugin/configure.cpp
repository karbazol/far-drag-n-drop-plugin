/**
 * @file The file contains implementation of Config class
 *
 * $Id: configure.cpp 21 2008-03-30 15:39:30Z eleskine $
 */

#include "far.h"
#include "ddlng.h"
#include "utils.h"
#include "configure.hpp"
#include "dll.h"

#define getItemId(structType,structItem) ((int)&((structType*)0)->structItem / sizeof(InitDialogItem))

static long WINAPI ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
    switch(Msg)
    {
    case DN_BTNCLICK:
        {
            if(Param1 == 1)
            {
                if(Param2
                    && BSTATE_UNCHECKED == SendDlgMessage(hDlg, DM_GETCHECK, 2, 0)
                    && BSTATE_UNCHECKED == SendDlgMessage(hDlg, DM_GETCHECK, 3, 0)
                    && BSTATE_UNCHECKED == SendDlgMessage(hDlg, DM_GETCHECK, 4, 0)
                    && BSTATE_UNCHECKED == SendDlgMessage(hDlg, DM_GETCHECK, 5, 0)
                    && BSTATE_UNCHECKED == SendDlgMessage(hDlg, DM_GETCHECK, 6, 0)
                    )
                {
                    SendDlgMessage(hDlg, DM_SETCHECK, 2, BSTATE_CHECKED);
                }
                SendDlgMessage(hDlg, DM_ENABLE, 2, Param2);
                SendDlgMessage(hDlg, DM_ENABLE, 3, Param2);
                SendDlgMessage(hDlg, DM_ENABLE, 4, Param2);
                SendDlgMessage(hDlg, DM_ENABLE, 5, Param2);
                SendDlgMessage(hDlg, DM_ENABLE, 6, Param2);
            }
        }
    }
    return DefDlgProc(hDlg, Msg, Param1, Param2);
}

int WINAPI Configure(int /*Number*/)
{
    static struct MyItems{
        InitDialogItem configTitle;
        InitDialogItem checkKeyToStartDrag;
        InitDialogItem radioLeftCtl;
        InitDialogItem radioLeftAlt;
        InitDialogItem radioShift;
        InitDialogItem radioRightCtl;
        InitDialogItem radioRightAlt;
        InitDialogItem sepPanels;
        InitDialogItem checkEnableDrop;
        InitDialogItem edtPixelsPassed;
        InitDialogItem txtPixelsPassed0;
        InitDialogItem txtPixelsPassed1;
        InitDialogItem sepEditor;
        InitDialogItem checkEnableDragEditor;
        InitDialogItem chackEnableDropEditor;
        InitDialogItem setButtons;

        InitDialogItem btnOk;
        InitDialogItem btnCancel;
    } Items =
    {
        /* 00 */{DI_DOUBLEBOX,3,1,46,15,0,0,0,0,(char*)MConfigTitle},
        /* 01 */{DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)MUseKeyToStartDND},
        /* 02 */{DI_RADIOBUTTON,5,3,0,0,0,0,(unsigned int)DIF_GROUP|DIF_DISABLE,0,(char *)MLeftCtl},
        /* 03 */{DI_RADIOBUTTON,5,4,0,0,0,0,(unsigned int)DIF_DISABLE,0,(char *)MLeftAlt},
        /* 04 */{DI_RADIOBUTTON,5,5,0,0,0,0,(unsigned int)DIF_DISABLE,0,(char *)MShift},
        /* 05 */{DI_RADIOBUTTON,22,3,0,0,0,0,(unsigned int)DIF_DISABLE,0,(char *)MRightCtl},
        /* 06 */{DI_RADIOBUTTON,22,4,0,0,0,0,(unsigned int)DIF_DISABLE,0,(char *)MRightAlt},
        {DI_TEXT,5,6,0,0,0,0,(unsigned int)DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERGROUP,0,(char*)MPanels},
            /* 08 */{DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MEnableDrop},
            /* 09 */{DI_EDIT,5,8,7,0,0,0,0,0,"0"},
            /* 10 */{DI_TEXT,9,8,37,0,0,0,0,0,(char *)MPixelsPassed},
            /* 11 */{DI_TEXT,9,9,37,0,0,0,0,0,(char *)MPixelsPassed2},
        {DI_TEXT,5,10,0,0,0,0,(unsigned int)DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERGROUP,0,(char*)MEditor},
        {DI_CHECKBOX,5,11,0,0,0,0,(unsigned int)DIF_DISABLE,0,(char *)MEnableDrag},
        {DI_CHECKBOX,5,12,0,0,0,0,(unsigned int)DIF_DISABLE,0,(char *)MEnableDrop},

        // ------- Buttons -------
        {DI_TEXT,5,13,0,0,0,0,(unsigned int)DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
        {DI_BUTTON,0,14,0,0,0,0,(unsigned int)DIF_CENTERGROUP,1,(char *)MOK},
        {DI_BUTTON,0,14,0,0,0,0,(unsigned int)DIF_CENTERGROUP,0,(char *)MCancel}
    };

    const size_t itemsCount = sizeof(Items)/sizeof(InitDialogItem);
    struct FarDialogItem DialogItems[itemsCount];

    InitDialogItems(&Items.configTitle, DialogItems, itemsCount);

    int checkKey = Config::instance()->checkKey();

    if(checkKey)
    {
        DialogItems[getItemId(MyItems,checkKeyToStartDrag)].Selected = 1;
        if(checkKey == LEFT_CTRL_PRESSED)
            DialogItems[getItemId(MyItems,radioLeftCtl)].Selected = 1;
        else if(checkKey == LEFT_ALT_PRESSED)
            DialogItems[getItemId(MyItems,radioLeftAlt)].Selected = 1;
        else if(checkKey == SHIFT_PRESSED)
            DialogItems[getItemId(MyItems,radioShift)].Selected = 1;
        else if(checkKey == RIGHT_CTRL_PRESSED)
            DialogItems[getItemId(MyItems,radioRightCtl)].Selected = 1;
        else if(checkKey == RIGHT_ALT_PRESSED)
            DialogItems[getItemId(MyItems,radioRightAlt)].Selected = 1;
        else
            DialogItems[getItemId(MyItems,checkKeyToStartDrag)].Selected = 0;

        if(DialogItems[getItemId(MyItems,checkKeyToStartDrag)].Selected)
        {
            DialogItems[getItemId(MyItems,radioLeftCtl)].Flags &= ~DIF_DISABLE;
            DialogItems[getItemId(MyItems,radioLeftAlt)].Flags &= ~DIF_DISABLE;
            DialogItems[getItemId(MyItems,radioShift)].Flags &= ~DIF_DISABLE;
            DialogItems[getItemId(MyItems,radioRightCtl)].Flags &= ~DIF_DISABLE;
            DialogItems[getItemId(MyItems,radioRightAlt)].Flags &= ~DIF_DISABLE;
        }
    }

    if(Config::instance()->allowDrop())
        DialogItems[getItemId(MyItems,checkEnableDrop)].Selected = 1;

    LONG_PTR nExitCode = DialogEx(-1, -1, DialogItems->X2 + 4, DialogItems->Y2 + 2,
        "Config", DialogItems, itemsCount, 0, 0, ConfigDlgProc, 0);
    if(nExitCode != getItemId(MyItems,btnOk))
        return FALSE;

    if(DialogItems[getItemId(MyItems,checkKeyToStartDrag)].Selected)
    {
        if(DialogItems[getItemId(MyItems,radioLeftCtl)].Selected)
            checkKey = LEFT_CTRL_PRESSED;
        else if(DialogItems[getItemId(MyItems,radioLeftAlt)].Selected)
            checkKey = LEFT_ALT_PRESSED;
        else if(DialogItems[getItemId(MyItems,radioShift)].Selected)
            checkKey = SHIFT_PRESSED;
        else if(DialogItems[getItemId(MyItems,radioRightCtl)].Selected)
            checkKey = RIGHT_CTRL_PRESSED;
        else if(DialogItems[getItemId(MyItems,radioRightAlt)].Selected)
            checkKey = RIGHT_ALT_PRESSED;
    }
    else
        checkKey = 0;

    Config::instance()->checkKey(checkKey);

    Config::instance()->allowDrop(DialogItems[getItemId(MyItems,checkEnableDrop)].Selected?true:false);

    if(Config::instance()->allowDrop())
    {
        // RegisterHooker();
    }
    else
    {
        // DeregisterHooker();
    };

    return TRUE;
}

Config::Config():_checkKey(0), _allowDrop(true), _shellCopy(true)
{
    _checkKey = FarReadRegistry(_checkKeyName, _checkKey);
    _allowDrop = FarReadRegistry(_allowDropName, _allowDrop)?true:false;
    _shellCopy = FarReadRegistry(_shellCopyName, _shellCopy)?true:false;
}

const char* Config::_allowDropName = "AllowDrop";
const char* Config::_checkKeyName = "CheckKey";
const char* Config::_shellCopyName = "UseShellCopy";

Config* Config::instance()
{
    static Config* p = 0;
    if (!p)
    {
        p = new Config();
        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
    }

    return p;
}

void Config::kill(Config* p)
{
    delete p;
}

void Config::checkKey(DWORD value)
{
    if (value != _checkKey)
    {
        _checkKey = value;

        FarWriteRegistry(_checkKeyName, _checkKey);
    }
}

void Config::allowDrop(bool value)
{
    if (value != _allowDrop)
    {
        _allowDrop = value;

        FarWriteRegistry(_allowDropName, _allowDrop);
    }
}

void Config::shellCopy(bool value)
{
    if (value != _shellCopy)
    {
        _shellCopy = value;
    }
}

// vim: set et ts=4 ai :

