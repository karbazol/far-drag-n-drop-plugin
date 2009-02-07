/**
 * @file configure.cpp
 * The file contains implementation of Config class
 *
 * $Id: configure.cpp 75 2008-10-02 17:51:35Z eleskine $
 */

#include "fardlg.h"
#include "ddlng.h"
#include "utils.h"
#include "configure.hpp"
#include "dll.h"

/**
 * @brief Configuration dialog items.
 * 
 * Represents configuration dialog items
 */
struct ConfigDlgItems
{
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
};

/**
 * Utility macro used to determine id of dialog item
 */
#define getMyItemId(i) getFarDlgItemId(ConfigDlgItems,i)

/**
 * @brief Configuration dialog
 */
class ConfigDlg: public FarDialog
{
public:
    static ConfigDlgItems _items;
    ConfigDlg(): FarDialog(){}
    ~ConfigDlg(){}
protected:
    long handle(int msg, int param1, long param2)
    {
        switch (msg)
        {
        case DN_BTNCLICK:
            {
                if (param1 == getMyItemId(checkKeyToStartDrag))
                    enableUseKeyToStartDnd(param2?getKeyToStartDnd():0);
            }
            break;
        }
        return FarDialog::handle(msg, param1, param2);
    }

    InitDialogItem* items()
    {
        return &_items.configTitle;
    }

    int itemsCount()
    {
        return sizeof(ConfigDlgItems)/sizeof(InitDialogItem);
    }
    int right()
    {
        return _items.configTitle.X2+4;
    }
    int bottom()
    {
        return _items.configTitle.Y2+2;
    }

    friend int WINAPI Configure(int);

public:
    void enableUseKeyToStartDnd(int value)
    {
        if (!value)
        {
            switchCheckBox(getMyItemId(checkKeyToStartDrag), BSTATE_UNCHECKED);
            disable(getMyItemId(radioLeftCtl) );
            disable(getMyItemId(radioLeftAlt) );
            disable(getMyItemId(radioShift)   );
            disable(getMyItemId(radioRightCtl));
            disable(getMyItemId(radioRightAlt));
        }
        else
        {
            switchCheckBox(getMyItemId(checkKeyToStartDrag), BSTATE_CHECKED);
            
            int key;
            switch (value)
            {
            case LEFT_CTRL_PRESSED:
                key = getMyItemId(radioLeftCtl);
                break;
            case LEFT_ALT_PRESSED:
                key = getMyItemId(radioLeftAlt);
                break;
            case SHIFT_PRESSED:
                key = getMyItemId(radioShift);
                break;
            case RIGHT_CTRL_PRESSED:
                key = getMyItemId(radioRightCtl);
                break;
            case RIGHT_ALT_PRESSED:
                key = getMyItemId(radioRightAlt);
                break;
            default:
                key = getMyItemId(radioLeftCtl);
            }
            
            enable(key);
            switchCheckBox(key, BSTATE_CHECKED);

            int i;
            for (i = getMyItemId(radioLeftCtl); i < key ; i++)
            {
                enable(i);
                switchCheckBox(i, BSTATE_UNCHECKED);
            }

            for (i = key + 1; i <= getMyItemId(radioRightAlt); i++)
            {
                enable(i);
                switchCheckBox(i, BSTATE_UNCHECKED);
            }
            
        }
    }

    int getKeyToStartDnd()
    {
        if (!checked(getMyItemId(checkKeyToStartDrag)))
            return 0;
        else if (checked(getMyItemId(radioLeftCtl)))
            return LEFT_CTRL_PRESSED;
        else if (checked(getMyItemId(radioLeftAlt)))
            return LEFT_ALT_PRESSED;
        else if (checked(getMyItemId(radioShift)))
            return SHIFT_PRESSED;
        else if (checked(getMyItemId(radioRightCtl)))
            return RIGHT_CTRL_PRESSED;
        else if (checked(getMyItemId(radioRightAlt)))
            return RIGHT_ALT_PRESSED;
        else
            return LEFT_CTRL_PRESSED;
    }
};

ConfigDlgItems ConfigDlg::_items =
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

/**
 * Callback function called by Far to show the plug-in's configuration dialog.
 */
int WINAPI Configure(int /*Number*/)
{
    ConfigDlg dlg;

    dlg.enableUseKeyToStartDnd(Config::instance()->checkKey());
    dlg.switchCheckBox(getMyItemId(checkEnableDrop),
            Config::instance()->allowDrop()?BSTATE_CHECKED:BSTATE_UNCHECKED);

    if (getMyItemId(btnOk) != dlg.show(true))
        return FALSE;

    Config::instance()->checkKey(dlg.getKeyToStartDnd());
    Config::instance()->allowDrop(dlg.checked(getMyItemId(checkEnableDrop)));

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

const wchar_t* Config::_allowDropName = L"AllowDrop";
const wchar_t* Config::_checkKeyName = L"CheckKey";
const wchar_t* Config::_shellCopyName = L"UseShellCopy";

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

        FarWriteRegistry(_shellCopyName, _shellCopy);
    }
}

// vim: set et ts=4 ai :

