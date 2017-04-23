/**
 * @file far.cpp
 * The file contains implementation of Far API.
 *
 * $Id: far_u.cpp 77 2011-10-22 20:25:46Z Karbazol $
 */

#include <math.h>
#include <shlwapi.h>

#include <utils.h>
#include <dll.h>
#include <mystring.h>
#ifndef PRODUCT_VERSION
#include <version.h>
#endif

#include "ddlng.h"

#include "configure.hpp"
#include "dndguids.h"
#include "dragging.h"
#include "far.h"
#include "hookfuncs.h"
#include "mainthrd.h"
#include "thrdpool.h"

static struct PluginStartupInfo theFar={0};
static struct FarStandardFunctions farFuncs={0};

#define __PLUGIN_VERSION(x) MAKEFARVERSION(##x,VS_RELEASE)
#define PLUGIN_VERSION() __PLUGIN_VERSION(PRODUCT_VERSION)

void WINAPI GetGlobalInfoW(
  struct GlobalInfo *Info
)
{
    Info->StructSize = sizeof(*Info);
    Info->MinFarVersion = MAKEFARVERSION(3,0,0,2927,VS_RELEASE);
    Info->Version = PLUGIN_VERSION();
    Info->Guid = pluginGuid;
    Info->Title = L"Dragndrop";
    Info->Description = L"Allows to drag files from and drop them on to FAR manager";
    Info->Author = L"Eugene Leskinen";
}

/**
 * Pointer to function which allow to check wheather this Far
 * is the active one.
 */
BOOL (*IsActiveFar)() = [](){return TRUE;};

static void checkConman()
{
    HMODULE conman = GetModuleHandle(L"infis.dll");
    if (conman)
    {
        void* IsConsoleActive = GetProcAddress(conman, "IsConsoleActive");
        if (IsConsoleActive)
        {
            IsActiveFar = reinterpret_cast<BOOL(*)()>(IsConsoleActive);
        }
    }
}

/**
 * Far calls this function to provide its API to the plug-in.
 * The plugin here performs its initialization.
 */
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
    initializeStringLock();

    checkConman();

    if (!patchImports())
        return;

    // Create MainThread instance
    MainThread::instance();

    TRACE("Setting plugin startup info\n");
    theFar = *Info;
    farFuncs = *Info->FSF;
    theFar.FSF = &farFuncs;

    // Read config from the registry
    Config::instance();

    // Prepare for dragging
    Dragging::instance()->initialize();
}

/**
 * Far calls this function when it is about to exit.
 */
void WINAPI ExitFARW(
  const struct ExitInfo *Info
)
{
    ThreadPool::instance()->shutDown();
    Dragging::instance()->shutDown();
    restoreImports();
}

/**
 * Far calls this function to get the plug-in's menu's strings
 */
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
    Info->StructSize = sizeof(*Info);
    Info->Flags = PF_PRELOAD;

    static const wchar_t* ConfigStrings[] = {
        L"", // GetMsg(MConfigMenu)
    };
    ConfigStrings[0] = GetMsg(MConfigMenu);
    Info->PluginConfig.Guids = &configGuid;
    Info->PluginConfig.Strings = ConfigStrings;
    Info->PluginConfig.Count = LENGTH(ConfigStrings);
}

intptr_t WINAPI ConfigureW(
  const struct ConfigureInfo *Info
)
{
    return doConfigure(0);
}

/**
 * Far's default dialog procedure
 */
LONG_PTR WINAPI DefDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
    if (!theFar.DefDlgProc)
    {
        return -1;
    }
    else
    {
        return theFar.DefDlgProc(hDlg, Msg, Param1, Param2);
    }
}

/**
 * Far's function to show a dialog
 */
HANDLE DialogInit(const GUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, size_t ItemsNumber,
  intptr_t Reserved, FARDIALOGFLAGS Flags, FARWINDOWPROC DlgProc, void* Param=NULL)
{
    HANDLE theDialog = INVALID_HANDLE_VALUE;
    if (theFar.DialogInit)
    {
        theDialog = theFar.DialogInit(&pluginGuid, Id, X1, Y1, X2, Y2,
                HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
        return theDialog;
    }

    return INVALID_HANDLE_VALUE;
}

/**
 * Far's function to run a dialog
 */
intptr_t DialogRun(HANDLE dlg)
{
    if (theFar.DialogRun)
    {
        return theFar.DialogRun(dlg);
    }

    return -1;
}

/**
 * Far's function to free a dialog
 */

void DialogFree(HANDLE h)
{
    if (theFar.DialogFree)
    {
        theFar.DialogFree(h);
    }
}

// Far standard functions

/**
 * Reads a message from the plug-in's language file
 */
const wchar_t* GetMsg(int MsgId)
{
    if (theFar.GetMsg)
    {
        return theFar.GetMsg(&pluginGuid, MsgId);
    }

    return 0;
}

/**
 * Sends a message to specified dialog
 */
intptr_t SendDlgMessage(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
    if (theFar.SendDlgMessage)
        return theFar.SendDlgMessage(hDlg, Msg, Param1, Param2);

    return -1;
}

/**
 * Gets window info from the Far.
 */
bool FarGetWindowInfo(struct WindowInfo& wi)
{
    if (theFar.AdvControl)
        return theFar.AdvControl(&pluginGuid, ACTL_GETWINDOWINFO, 0, &wi)?true:false;

    return false;
}

/**
* Gets Panel info from the Far.
*/
static bool FarGetPanelInfo(HANDLE hPanel, struct PanelInfo& pi)
{
    if (!theFar.PanelControl)
        return false;
    if (!theFar.PanelControl(hPanel, FCTL_GETPANELINFO, 0, &pi))
        return false;
    SMALL_RECT farRect;
    if (!theFar.AdvControl || !theFar.AdvControl(&pluginGuid, ACTL_GETFARRECT, 0, &farRect)) {
        farRect.Left = 0;
        farRect.Top = 0;
    }
    pi.PanelRect.left += farRect.Left;
    pi.PanelRect.top += farRect.Top;
    pi.PanelRect.right += farRect.Left;
    pi.PanelRect.bottom += farRect.Top;
    return true;
}

/**
 * Gets Active Panel info from the Far.
 */
bool FarGetActivePanelInfo(struct PanelInfo& pi)
{
    return FarGetPanelInfo(PANEL_ACTIVE, pi);
}

static PluginPanelItemW& copy(PluginPanelItemW& pw, const PluginPanelItem& pi)
{
    pw.CreationTime = pi.CreationTime;
    pw.LastAccessTime = pi.LastAccessTime;
    pw.LastWriteTime = pi.LastWriteTime;
    pw.ChangeTime = pi.ChangeTime;

    pw.FileSize = pi.FileSize;
    pw.AllocationSize = pi.AllocationSize;

    pw.FileName = pi.FileName;
    pw.AlternateFileName = pi.AlternateFileName;
    pw.Description = pi.Description;
    pw.Owner = pi.Owner;

    pw.Flags = pi.Flags;
    pw.FileAttributes = pi.FileAttributes;
    pw.NumberOfLinks = pi.NumberOfLinks;

    pw.UserData.Data = pi.UserData.Data;
    pw.UserData.FreeData = pi.UserData.FreeData;

    pw.CRC32 = pi.CRC32;
    pw.Reserved[0] = pi.Reserved[0];
    pw.Reserved[1] = pi.Reserved[1];

    pw.CustomColumnData.size(pi.CustomColumnNumber);

    for (size_t i = 0; i < pi.CustomColumnNumber; i++)
    {
        pw.CustomColumnData[i] = pi.CustomColumnData[i];
    }

    return pw;
}

bool FarGetPassivePanelInfo(PanelInfo& pi)
{
    return FarGetPanelInfo(PANEL_PASSIVE, pi);
}

static MyStringW FarGetPanelDirectory(bool activePanel)
{
    MyStringW res;

    if (theFar.PanelControl)
    {
        HANDLE h = activePanel?PANEL_ACTIVE:PANEL_PASSIVE;

        size_t buffSize = theFar.PanelControl(h, FCTL_GETPANELDIRECTORY, 0, 0);

        FarPanelDirectory* buff = reinterpret_cast<FarPanelDirectory*>(malloc(buffSize));
        if (buff)
        {
            buff->StructSize = sizeof(*buff);
            theFar.PanelControl(h, FCTL_GETPANELDIRECTORY, static_cast<intptr_t>(buffSize), buff);

            res = buff->Name;

            free(buff);
        }
    }

    return res;
}

MyStringW FarGetActivePanelDirectory()
{
    return FarGetPanelDirectory(true);
}

MyStringW FarGetPassivePanelDirectory()
{
    return FarGetPanelDirectory(false);
}

static bool FarGetPanelItem(HANDLE panel, FILE_CONTROL_COMMANDS command, size_t itemIndex, PluginPanelItemW& item)
{
    intptr_t size = theFar.PanelControl(panel, command, itemIndex, 0);

    if (size < static_cast<intptr_t>(sizeof(PluginPanelItem)))
    {
        return false;
    }

    FarGetPluginPanelItem getItem = {
        sizeof(getItem),
        static_cast<size_t>(size),
        reinterpret_cast<PluginPanelItem*>(malloc(size))
    };

    if (!getItem.Item)
    {
        return false;
    }

    theFar.PanelControl(panel, command, static_cast<intptr_t>(itemIndex), &getItem);

    copy(item, *getItem.Item);

    free(getItem.Item);

    return true;
}

PluginPanelItemsW FarGetPanelItems(bool active, bool selected)
{
    PluginPanelItemsW res;

    if (theFar.PanelControl)
    {
        HANDLE h = active?PANEL_ACTIVE:PANEL_PASSIVE;
        FILE_CONTROL_COMMANDS command = selected?FCTL_GETSELECTEDPANELITEM:FCTL_GETPANELITEM;

        PanelInfo pi = {sizeof(pi)};

        if (theFar.PanelControl(h, FCTL_GETPANELINFO, 0, &pi))
        {
            size_t count = selected?pi.SelectedItemsNumber:pi.ItemsNumber;

            if (count > 0)
            {
                res.size(count);

                if (count > 1)
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        FarGetPanelItem(h, command, i, res[i]);

                    }
                }
                else
                {
                    FarGetPanelItem(h, command, selected?0:pi.CurrentItem, res[0]);
                }
            }
        }
    }

    return res;
}


PluginPanelItemsW FarGetActivePanelItems(bool selected)
{
    return FarGetPanelItems(true, selected);
}

PluginPanelItemsW FarGetPassivePanelItems(bool selected)
{
    return FarGetPanelItems(false, selected);
}

bool FarClearSelectionActivePanel(intptr_t from, intptr_t count)
{
    if (!theFar.PanelControl||!theFar.PanelControl(
                PANEL_ACTIVE, FCTL_BEGINSELECTION, 0, nullptr))
    {
        return false;
    }
    bool res = true;
    for (intptr_t i = 0; i < count; i++)
    {
        if (!theFar.PanelControl(PANEL_ACTIVE, FCTL_CLEARSELECTION, from++, nullptr))
        {
            res = false;
            break;
        }
    }
    theFar.PanelControl(PANEL_ACTIVE, FCTL_ENDSELECTION, 0, nullptr);
    theFar.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, nullptr);
    return res;
}

/**
 * Returns a handle to the Far console window.
 */
HWND GetFarWindow()
{
    static HWND hwnd = 0;
    if (!hwnd)
    {
        if (!theFar.AdvControl)
            return NULL;

        hwnd = (HWND)theFar.AdvControl(&pluginGuid, ACTL_GETFARHWND, 0, NULL);

        // The case of ConEmu
        HWND parent = GetAncestor(hwnd, GA_PARENT);
        if (parent != 0)
        {
            // Far's parent is not null.
            // This may mean far is run in ConEmu not in generic console
            wchar_t buff[260];
            if (GetClassName(parent, buff, LENGTH(buff)))
            {
                TRACE("Far parent windows is %S\n", buff);
                if (!lstrcmp(buff, L"VirtualConsoleClass"))
                {
                    TRACE("Far is running under conemu\n");
                    hwnd = parent;
                }
            }
        }
    }

    return hwnd;
}

static FARSETTINGSTYPES regTypeToFarType(DWORD type)
{
    switch (type)
    {
    case REG_BINARY:
        return FST_DATA;
    case REG_DWORD:
    case REG_DWORD_BIG_ENDIAN:
    case REG_QWORD:
        return FST_QWORD;
    case REG_EXPAND_SZ:
    case REG_LINK:
    case REG_MULTI_SZ:
    case REG_SZ:
        return FST_STRING;
    case REG_NONE:
    default:
        return FST_UNKNOWN;
    }
}

static DWORD farTypeToRegType(FARSETTINGSTYPES type)
{
    switch (type)
    {
    case FST_QWORD:
        return REG_QWORD;
    case FST_STRING:
        return REG_SZ;
    case FST_DATA:
        return REG_BINARY;
    case FST_UNKNOWN:
    case FST_SUBKEY:
    default:
        return REG_NONE;
    }
}

/**
 * Writes data to the registry
 */
bool FarWriteRegistry(const wchar_t* name, DWORD type, const void* value, size_t size)
{
    if (!theFar.SettingsControl) {
        return false;
    }
    FarSettingsCreate settings = {sizeof(settings), pluginGuid};
    if (!theFar.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &settings)) {
        return false;
    }

    FarSettingsItem data = {
        sizeof(data),
        0, // Root
        name,
        regTypeToFarType(type)
    };
    switch (data.Type)
    {
    case FST_QWORD:
        memmove(&data.Number, value, min(size, sizeof(data.Number)));
        break;
    case FST_STRING:
        data.String = reinterpret_cast<const wchar_t*>(value);
        break;
    case FST_DATA:
        data.Data.Data = value;
        data.Data.Size = size;
        break;
    }
    bool res = !!theFar.SettingsControl(settings.Handle, SCTL_SET, 0, &data);

    theFar.SettingsControl(settings.Handle, SCTL_FREE, 0, 0);

    return res;
}

/**
 * Writes data to the registry
 */
bool FarWriteRegistry(const wchar_t* name, const DWORD value)
{
    return FarWriteRegistry(name, REG_DWORD, (const BYTE*)&value, sizeof(value));
}

/**
 * Reads data from the registry
 */
size_t FarReadRegistry(const wchar_t* name, DWORD type, void* value, size_t size)
{
    if (!theFar.SettingsControl) {
        return false;
    }
    FarSettingsCreate settings = {sizeof(settings), pluginGuid};
    if (!theFar.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &settings)) {
        return false;
    }

    FarSettingsItem data = {
        sizeof(data),
        0, // Root
        name,
        regTypeToFarType(type)
    };

    size_t res = theFar.SettingsControl(settings.Handle, SCTL_GET, 0, &data);
    if (res) {
        if (regTypeToFarType(type) != data.Type)
        {
            res = 0;
        }
        else
        {
            switch (data.Type) {
            case FST_UNKNOWN:
            case FST_SUBKEY:
                break;
            case FST_QWORD:
                memmove(value, &data.Number, min(size, sizeof(data.Number)));
                res = sizeof(data.Number);
                break;
            case FST_STRING:
                memmove(value, data.String, res = min(size, static_cast<size_t>(lstrlen(data.String) + 1)));
                break;
            case FST_DATA:
                memmove(value, data.Data.Data, res = min(size, data.Data.Size));
                break;
            }
        }
    }

    theFar.SettingsControl(settings.Handle, SCTL_FREE, 0, 0);

    return res;
}

/**
 * Reads data from the registry
 */
DWORD FarReadRegistry(const wchar_t* name, DWORD defaultValue)
{
    DWORD res = defaultValue;

    FarReadRegistry(name, REG_DWORD, &res, sizeof(res));

    return res;
}

/**
 * Truncates a string.
 */
MyStringA TruncPathStr(const MyStringA& s, int maxLen)
{
    MyStringW sW = a2w(s, CP_OEMCP);

    return w2a(TruncPathStr(sW, maxLen), CP_OEMCP);
}

MyStringW TruncPathStr(const MyStringW& s, int maxLen)
{
    if (!theFar.FSF->TruncPathStr)
        return MyStringW(s);

    MyStringW res(s);
    return MyStringW(theFar.FSF->TruncPathStr(res, maxLen));
}

// vim: set et ts=4 sw=4 ai :

