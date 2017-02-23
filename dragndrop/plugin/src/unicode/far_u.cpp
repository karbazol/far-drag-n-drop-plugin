/**
 * @file far.cpp
 * The file contains implementation of Far API.
 *
 * $Id$
 */

#include <shlwapi.h>
#include <dll.h>
#include "../far.h"
#include "../configure.hpp"
#include "ddlng.h"
#include <utils.h>
#include "../hookfuncs.h"
#include "../dragging.h"
#include "../mainthrd.h"
#include "../thrdpool.h"
#include <mystring.h>

static struct PluginStartupInfo theFar={0};
static struct FarStandardFunctions farFuncs={0};

/**
 * Far calls this function to determine minimal version of Far
 * supported by the plug-in
 */
#ifndef FAR2
 // {453E81B7-43A7-4A95-8301-E4499A2695B5}
static const GUID PluginGuid =
{ 0x453e81b7, 0x43a7, 0x4a95,{ 0x83, 0x1, 0xe4, 0x49, 0x9a, 0x26, 0x95, 0xb5 } };

void WINAPI GetGlobalInfoW(GlobalInfo* info)
{
    info->StructSize = sizeof(*info);
    info->MinFarVersion = FARMANAGERVERSION;
    info->Version = MAKEFARVERSION(3, 0, 1, 0, VS_RELEASE);
    info->Guid = PluginGuid;
    info->Title = L"drag-n-drop";
    info->Description = L"This plugin enables drag'n'drop operations between Far and other apps.";
    info->Author = L"Karbazol";
}
#else
int WINAPI GetMinFarVersionW()
{
    return FARMANAGERVERSION;
}
#endif

static BOOL alwaysTrue()
{
    return TRUE;
}

/**
 * Pointer to function which allow to check wheather this Far
 * is the active one.
 */
BOOL (*IsActiveFar)() = 0;

static void checkConman()
{
    HMODULE conman = GetModuleHandle(L"infis.dll");
    if (conman)
    {
        IsActiveFar = (BOOL (*)())GetProcAddress(conman, "IsConsoleActive");
    }
    if (!IsActiveFar)
        IsActiveFar = &alwaysTrue;
}

#ifdef FAR2
static void freeRegistry(MyStringA* registry)
{
    delete registry;
}

static const wchar_t* getRegistryKeyName()
{
    static MyStringW* regKeyName = 0;
    if (!regKeyName)
    {
        regKeyName = new MyStringW(theFar.RootKey);
        *regKeyName /= L"Karbazol\\DragNDrop";

        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&freeRegistry), regKeyName);
    }

    return *regKeyName;
}
#endif

/**
 * Far calls this function to provide its API to the plug-in.
 * The plugin here performs its initialization.
 */
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
#ifndef FAR2
    if (Info->StructSize < sizeof(*Info))
        return;
#endif
    checkConman();

    if (!patchImports())
        return;

    // Create MainThread instance
    MainThread::instance();

    TRACE("Setting plugin startup info\n");
    theFar = *Info;
    farFuncs = *Info->FSF;
    theFar.FSF = &farFuncs;

#ifdef FAR2
    // Make sure we have correct registry key name
    getRegistryKeyName();
#endif

    // Read config from the registry
    Config::instance();

    // Prepare for dragging
    Dragging::instance()->initialize();
}

/**
 * Far calls this function when it is about to exit.
 */
void WINAPI ExitFARW(void)
{
#ifndef FAR2
    if (!theFar.StructSize) // early initialization failed
        return;
#endif
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

    static const wchar_t* ConfigStrings[1];
    ConfigStrings[0] = GetMsg(MConfigMenu);
#ifdef FAR2
    Info->PluginConfigStrings = ConfigStrings;
    Info->PluginConfigStringsNumber = LENGTH(ConfigStrings);
#else
    Info->PluginConfig.Count = 1;
    Info->PluginConfig.Strings = ConfigStrings;
    Info->PluginConfig.Guids = &PluginGuid;
#endif
}

#ifdef FAR2
int WINAPI ConfigureW(int)
#else
intptr_t WINAPI ConfigureW(const struct ConfigureInfo*)
#endif
{
    return doConfigure(0);
}

/**
 * Far's default dialog procedure
 */
FAR_RETURN_TYPE WINAPI DefDlgProc(HANDLE hDlg, FAR_WPARAM_TYPE Msg, FAR_WPARAM_TYPE Param1, FAR_LPARAM_TYPE Param2)
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
HANDLE DialogInit(const GUID* guid, int X1, int Y1, int X2, int Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, FAR_LPARAM_TYPE Param)
{
    HANDLE theDialog = INVALID_HANDLE_VALUE;
    if (theFar.DialogInit)
    {
#ifdef FAR2
        theDialog = theFar.DialogInit(theFar.ModuleNumber, X1, Y1, X2, Y2,
                HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
#else
        theDialog = theFar.DialogInit(&PluginGuid, guid, X1, Y1, X2, Y2,
            HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
#endif
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
#ifdef FAR2
        return theFar.GetMsg(theFar.ModuleNumber, MsgId);
#else
        return theFar.GetMsg(&PluginGuid, MsgId);
#endif
    }

    return 0;
}

/**
 * Sends a message to specified dialog
 */
FAR_RETURN_TYPE SendDlgMessage(HANDLE hDlg, FAR_WPARAM_TYPE Msg, FAR_WPARAM_TYPE Param1, FAR_LPARAM_TYPE Param2)
{
    if (theFar.SendDlgMessage)
        return theFar.SendDlgMessage(hDlg, Msg, Param1, Param2);

    return -1;
}

/**
 * Helps to determine wheather the Far is running in Full-screen mode or
 * in windowed mode
 */
int ConsoleMode(int /*param*/)
{
    return FAR_CONSOLE_WINDOWED;
}

bool FarGetWindowInfo(struct WindowInfoW& wi)
{
    if (!theFar.AdvControl)
        return false;
    WindowInfo info;
    info.TypeName = nullptr;
    info.Name = nullptr;
    info.TypeNameSize = 0;
    info.NameSize = 0;
#ifdef FAR2
    info.Pos = (int)wi.Pos;
    if (!theFar.AdvControl(theFar.ModuleNumber, ACTL_GETWINDOWINFO, &info))
        return false;
#else
    info.StructSize = sizeof(info);
    info.Pos = wi.Pos;
    if (!theFar.AdvControl(&PluginGuid, ACTL_GETWINDOWINFO, 0, &info))
        return false;
#endif
    wi.Pos = info.Pos;
    wi.Type = info.Type;
    return true;
}
/**
* Gets Panel info from the Far.
*/
static bool FarGetPanelInfo(HANDLE hPanel, PanelInfoW& pw)
{
    PanelInfo pi;
    SMALL_RECT farRect;
#ifdef FAR2
    if (!theFar.Control)
        return false;
    if (!theFar.Control(hPanel, FCTL_GETPANELINFO, 0, reinterpret_cast<LONG_PTR>(&pi)))
        return false;
    pw.Plugin = pi.Plugin;
    pw.Visible = pi.Visible;
    if (!theFar.AdvControl || !theFar.AdvControl(theFar.ModuleNumber, ACTL_GETPANELRECT, &farRect)) {
        farRect.Left = 0;
        farRect.Top = 0;
    }
#else
    pi.StructSize = sizeof(pi);
    if (!theFar.PanelControl)
        return false;
    if (!theFar.PanelControl(hPanel, FCTL_GETPANELINFO, 0, &pi))
        return false;
    pw.Plugin = (pi.Flags & PFLAGS_PLUGIN);
    pw.Visible = (pi.Flags & PFLAGS_VISIBLE);
    if (!theFar.AdvControl || !theFar.AdvControl(&PluginGuid, ACTL_GETFARRECT, 0, &farRect)) {
        farRect.Left = 0;
        farRect.Top = 0;
    }
#endif
    pw.PanelRect.left = pi.PanelRect.left + farRect.Left;
    pw.PanelRect.top = pi.PanelRect.top + farRect.Top;
    pw.PanelRect.right = pi.PanelRect.right + farRect.Left;
    pw.PanelRect.bottom = pi.PanelRect.bottom + farRect.Top;
    pw.Flags = (DWORD)pi.Flags;
    return true;
}
bool FarGetActivePanelInfo(PanelInfoW& pw)
{
    return FarGetPanelInfo(PANEL_ACTIVE, pw);
}
bool FarGetPassivePanelInfo(PanelInfoW& pw)
{
    return FarGetPanelInfo(PANEL_PASSIVE, pw);
}
static MyStringW FarGetPanelDirectory(HANDLE hPanel)
{
    MyStringW res;
#ifdef FAR2
    if (theFar.Control)
    {
        size_t buffSize = theFar.Control(hPanel, FCTL_GETPANELDIR, 0, 0);

        wchar_t* buff = new wchar_t[buffSize];

        theFar.Control(hPanel, FCTL_GETPANELDIR, static_cast<int>(buffSize),
            reinterpret_cast<LONG_PTR>(static_cast<wchar_t*>(buff)));

        res = buff;

        delete[] buff;
    }
#else
    if (theFar.PanelControl)
    {
        size_t buffSize = theFar.PanelControl(hPanel, FCTL_GETPANELDIRECTORY, 0, 0);
        FarPanelDirectory* buff = (FarPanelDirectory*)malloc(buffSize);
        theFar.PanelControl(hPanel, FCTL_GETPANELDIRECTORY, buffSize, buff);
        res = buff->Name;
        free(buff);
    }
#endif
    return res;
}

MyStringW FarGetActivePanelDirectory()
{
    return FarGetPanelDirectory(PANEL_ACTIVE);
}

MyStringW FarGetPassivePanelDirectory()
{
    return FarGetPanelDirectory(PANEL_PASSIVE);
}

#ifdef FAR2
static bool FarGetPanelItem(HANDLE panel, int command, int itemIndex, PluginPanelItemW& item)
{
    int size = theFar.Control(panel, command, itemIndex, 0);

    if (size < static_cast<int>(sizeof(PluginPanelItem)))
    {
        return false;
    }

    PluginPanelItem* tmp = reinterpret_cast<PluginPanelItem*>(
        malloc(size));

    if (!tmp)
    {
        return false;
    }

    theFar.Control(panel, command, itemIndex, reinterpret_cast<LONG_PTR>(tmp));

    item.cFileName = tmp->FindData.lpwszFileName;

    free(tmp);

    return true;
}
PluginPanelItemsW FarGetPanelItems(HANDLE hPanel, bool selected)
{
    PluginPanelItemsW res;

    if (theFar.Control)
    {
        int command = selected ? FCTL_GETSELECTEDPANELITEM : FCTL_GETPANELITEM;

        PanelInfo pi;

        if (theFar.Control(hPanel, FCTL_GETPANELINFO, 0, reinterpret_cast<LONG_PTR>(&pi)))
        {
            int count = selected ? pi.SelectedItemsNumber : pi.ItemsNumber;

            if (count > 0)
            {
                res.size(count);

                if (count > 1)
                {
                    int i;

                    for (i = 0; i < count; ++i)
                    {
                        FarGetPanelItem(hPanel, command, i, res[i]);

                    }
                } else
                {
                    FarGetPanelItem(hPanel, command, selected ? 0 : pi.CurrentItem, res[0]);
                }
            }
        }
    }

    return res;
}
#else
static bool FarGetPanelItem(HANDLE panel, FILE_CONTROL_COMMANDS command, size_t itemIndex, PluginPanelItemW& item)
{
    size_t size = theFar.PanelControl(panel, command, itemIndex, 0);

    if (size < sizeof(PluginPanelItem))
    {
        return false;
    }

    PluginPanelItem* tmp = reinterpret_cast<PluginPanelItem*>(
        malloc(size));

    if (!tmp)
    {
        return false;
    }

    FarGetPluginPanelItem ppi = { sizeof(ppi), size, tmp };
    if (!theFar.PanelControl(panel, command, itemIndex, &ppi))
        return false;

    item.cFileName = tmp->FileName;

    free(tmp);

    return true;
}
PluginPanelItemsW FarGetPanelItems(HANDLE hPanel, bool selected)
{
    PluginPanelItemsW res;

    if (theFar.PanelControl)
    {
        FILE_CONTROL_COMMANDS command = selected ? FCTL_GETSELECTEDPANELITEM : FCTL_GETPANELITEM;

        PanelInfo pi;
        pi.StructSize = sizeof(pi);

        if (theFar.PanelControl(hPanel, FCTL_GETPANELINFO, 0, &pi))
        {
            size_t count = selected ? pi.SelectedItemsNumber : pi.ItemsNumber;

            if (count > 0)
            {
                res.size(count);

                if (count > 1)
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        FarGetPanelItem(hPanel, command, i, res[i]);

                    }
                } else
                {
                    FarGetPanelItem(hPanel, command, selected ? 0 : pi.CurrentItem, res[0]);
                }
            }
        }
    }

    return res;
}
#endif

PluginPanelItemsW FarGetActivePanelItems(bool selected)
{
    return FarGetPanelItems(PANEL_ACTIVE, selected);
}

PluginPanelItemsW FarGetPassivePanelItems(bool selected)
{
    return FarGetPanelItems(PANEL_PASSIVE, selected);
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

#ifdef FAR2
        hwnd = (HWND)theFar.AdvControl(theFar.ModuleNumber, ACTL_GETFARHWND, NULL);
#else
        hwnd = (HWND)theFar.AdvControl(&PluginGuid, ACTL_GETFARHWND, 0, NULL);
#endif

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

#ifdef FAR2
/**
 * Writes data to the registry
 */
static bool FarWriteRegistry(const wchar_t* name, DWORD type, const void* value, size_t size)
{
    HKEY key = 0;

    if (ERROR_SUCCESS != RegCreateKeyExW(HKEY_CURRENT_USER, getRegistryKeyName(), 0,
            NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL))
        return false;

    bool res = (ERROR_SUCCESS == RegSetValueExW(key, name, 0, type,
                reinterpret_cast<const BYTE*>(value), static_cast<DWORD>(size)));

    RegCloseKey(key);

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
static size_t FarReadRegistry(const wchar_t* name, DWORD type, void* value, size_t size)
{
    HKEY key = 0;

    if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, getRegistryKeyName(),
                0, KEY_READ, &key))
        return 0;

    DWORD res = static_cast<DWORD>(size);

    if (ERROR_SUCCESS != RegQueryValueExW(key, name, NULL, &type, (BYTE*)value, &res))
        res = 0;

    RegCloseKey(key);

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
#else
/**
* Writes data to the registry
*/
bool FarWriteRegistry(const wchar_t* name, const DWORD value)
{
    FarSettingsCreate settingsCreate;
    settingsCreate.StructSize = sizeof(settingsCreate);
    settingsCreate.Guid = PluginGuid;
    if (!theFar.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &settingsCreate))
        return false;
    FarSettingsItem item;
    item.StructSize = sizeof(item);
    item.Root = 0;
    item.Name = name;
    item.Type = FST_QWORD;
    item.Number = value;
    bool ret = (theFar.SettingsControl(settingsCreate.Handle, SCTL_SET, 0, &item) != FALSE);
    theFar.SettingsControl(settingsCreate.Handle, SCTL_FREE, 0, nullptr);
    return ret;
}

/**
* Reads data from the registry
*/
DWORD FarReadRegistry(const wchar_t* name, DWORD defaultValue)
{
    FarSettingsCreate settingsCreate;
    settingsCreate.StructSize = sizeof(settingsCreate);
    settingsCreate.Guid = PluginGuid;
    if (!theFar.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &settingsCreate))
        return defaultValue;
    FarSettingsItem item;
    item.StructSize = sizeof(item);
    item.Root = 0;
    item.Name = name;
    item.Type = FST_QWORD;
    bool ret = (theFar.SettingsControl(settingsCreate.Handle, SCTL_GET, 0, &item) != FALSE);
    theFar.SettingsControl(settingsCreate.Handle, SCTL_FREE, 0, nullptr);
    if (ret) {
        return (DWORD)item.Number;
    }
    return defaultValue;
}
#endif

MyStringW TruncPathStr(const MyStringW& s, int maxLen)
{
    if (!theFar.FSF->TruncPathStr)
        return MyStringW(s);

    MyStringW res(s);
    return MyStringW(theFar.FSF->TruncPathStr(res, maxLen));
}

// vim: set et ts=4 sw=4 ai :
