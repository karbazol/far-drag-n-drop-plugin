/**
 * @file far.cpp
 * The file contains implementation of Far API.
 *
 * $Id$
 */

#include <shlwapi.h>
#include "dll.h"
#include "far.h"
#include "configure.hpp"
#include "ddlng.h"
#include "utils.h"
#include "hookfuncs.h"
#include "dragging.h"
#include "mainthrd.h"
#include "thrdpool.h"
#include "mystring.h"

static struct PluginStartupInfo theFar={0};
static struct FarStandardFunctions farFuncs={0};

/**
 * Far calls this function to determine minimal version of Far
 * supported by the plug-in
 */
int WINAPI GetMinFarVersion()
{
    return FARMANAGERVERSION;
}

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
    {
        IsActiveFar = &alwaysTrue;
    }
}

static void freeRegistry(MyStringA* registry)
{
    delete registry;
}

static const wchar_t* getRegistryKeyName()
{
    static MyStringW* regKeyName = 0;
    if (!regKeyName)
    {
        regKeyName = new MyStringW(a2w(theFar.RootKey, CP_OEMCP));
        if (regKeyName)
        {
            *regKeyName /= L"Karbazol\\DragNDrop";

            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&freeRegistry), regKeyName);
            }
        }
    }

    return regKeyName?*regKeyName:0;
}

/**
 * Far calls this function to provide its API to the plug-in.
 * The plugin here performs its initialization.
 */
void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
{
    checkConman();

    if (!patchImports())
    {
        return;
    }

    // Create MainThread instance
    if (!MainThread::instance())
    {
        // if failed to create MainThread class instance all
        // the follwing does not make sence.
        return;
    }

    TRACE("Setting plugin startup info\n");
    theFar = *Info;
    farFuncs = *Info->FSF;
    theFar.FSF = &farFuncs;

    // Make sure we have correct registry key name
    getRegistryKeyName();

    // Read config from the registry
    Config::instance();

    // Prepare for dragging
    Dragging* dragging = Dragging::instance();
    if (dragging)
    {
        dragging->initialize();
    }
}

/**
 * Far calls this function when it is about to exit.
 */
void WINAPI ExitFAR(void)
{
    ThreadPool::instance()->shutDown();
    Dragging::instance()->shutDown();
    restoreImports();
}

/**
 * Far calls this function to get the plug-in's menu's strings
 */
void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
    Info->StructSize = sizeof(*Info);
    Info->Flags = PF_PRELOAD;

    static const char* ConfigStrings[1];
    ConfigStrings[0] = theFar.GetMsg(theFar.ModuleNumber, MConfigMenu);
    Info->PluginConfigStrings = ConfigStrings;
    Info->PluginConfigStringsNumber = LENGTH(ConfigStrings);
}

int WINAPI Configure(int Number)
{
    return doConfigure(Number);
}
/**
 * Far's default dialog procedure
 */
LONG_PTR WINAPI DefDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
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
int DialogEx(int X1, int Y1, int X2, int Y2,
  const char *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, long Param)
{
    if (theFar.DialogEx)
    {
        return theFar.DialogEx(theFar.ModuleNumber, X1, Y1, X2, Y2,
                HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
    }

    return -1;
}

// Far standard functions

struct MessageInfo
{
    MyStringW wMessage;
    const char* aMessage;
    MessageInfo(): wMessage(), aMessage(0){}
};

typedef GrowOnlyArray<MessageInfo> Messages;

static void killMessages(Messages* p)
{
    delete p;
}

static Messages* messagesInstance()
{
    static Messages* m = 0;
    if (!m)
    {
        m = new Messages;

        if (m)
        {
            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(
                    reinterpret_cast<PdllCallBack>(&killMessages), m);
            }
        }
    }

    return m;
}

/**
 * Reads a message from the plug-in's language file
 */
const wchar_t* GetMsg(int MsgId)
{
    Messages* messages = messagesInstance();
    if (!messages)
    {
        return L"";
    }
    if (static_cast<unsigned int>(MsgId) >= messages->size())
    {
        messages->size(MsgId+1);
    }
    if (theFar.GetMsg)
    {
        const char* p = theFar.GetMsg(theFar.ModuleNumber, MsgId);
        if ((*messages)[MsgId].aMessage != p)
        {
            (*messages)[MsgId].aMessage = p;
            (*messages)[MsgId].wMessage = a2w(p, CP_OEMCP);
        }
    }

    return (*messages)[MsgId].wMessage;
}

/**
 * Sends a message to specified dialog
 */
LONG_PTR SendDlgMessage(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
    if (theFar.SendDlgMessage)
    {
        MyStringA tmp;
        char* tmpP;
        switch (Msg)
        {
        case DM_SETTEXTPTR:
            {
                tmp = w2a(reinterpret_cast<wchar_t*>(Param2), CP_OEMCP);
                tmpP = tmp;
                Param2 = reinterpret_cast<long>(static_cast<char*>(tmpP));
            }
            break;
        default:
            break;
        }
        return theFar.SendDlgMessage(hDlg, Msg, Param1, Param2);
    }

    return -1;
}

/**
 * Helps to determine wheather the Far is running in Full-screen mode or
 * in windowed mode
 */
int ConsoleMode(int param)
{
    if (theFar.AdvControl)
        return static_cast<int>(
                theFar.AdvControl(theFar.ModuleNumber, ACTL_CONSOLEMODE, (void*)param));
    return -1;
}

/**
 * Gets window info from the Far.
 */
bool FarGetWindowInfo(struct WindowInfo* wi)
{
    if (theFar.AdvControl)
        return theFar.AdvControl(theFar.ModuleNumber, ACTL_GETWINDOWINFO, wi)?true:false;

    return false;
}

static WindowInfoW& copy(WindowInfoW& wip, WindowInfo& wi)
{
    wip.Pos = wi.Pos;
    wip.Modified = wi.Modified;
    wip.Type = wi.Type;
    wip.Current = wi.Current;

    wip.TypeName = a2w(wi.TypeName, CP_OEMCP);
    wip.Name = a2w(wi.Name, CP_OEMCP);

    return wip;
}

bool FarGetWindowInfo(WindowInfoW& wip)
{
    struct WindowInfo wi;
    wi.Pos = wip.Pos;
    if (!FarGetWindowInfo(&wi))
        return false;

    copy(wip, wi);
    return true;
}
/**
 * Gets Panel info from the Far.
 */
static bool FarGetPanelInfo(struct PanelInfo* pi, bool active, bool full)
{
    int command;
    if (active)
    {
        command = full?FCTL_GETPANELINFO:FCTL_GETPANELSHORTINFO;
    }
    else
    {
        command = full?FCTL_GETANOTHERPANELINFO:FCTL_GETANOTHERPANELSHORTINFO;
    }

    if (theFar.Control)
        return theFar.Control(INVALID_HANDLE_VALUE, command, pi)?true:false;
    return false;
}

static FAR_FIND_DATA_W& copy(FAR_FIND_DATA_W& pw, const FAR_FIND_DATA& pi)
{
    pw.dwFileAttributes = pi.dwFileAttributes;
    pw.ftCreationTime = pi.ftCreationTime;
    pw.ftLastAccessTime = pi.ftLastAccessTime;
    pw.ftLastWriteTime = pi.ftLastWriteTime;
    pw.nFileSizeHigh = pi.nFileSizeHigh;
    pw.nFileSizeLow = pi.nFileSizeLow;
    pw.dwReserved0 = pi.dwReserved0;
    pw.dwReserved1 = pi.dwReserved1;

    pw.cFileName = a2w(pi.cFileName, CP_OEMCP);
    pw.cAlternateFileName = a2w(pi.cAlternateFileName, CP_OEMCP);

    return pw;
}

static PluginPanelItemW& copy(PluginPanelItemW& pw, const PluginPanelItem& pi)
{
    copy(pw.FindData, pi.FindData);

    pw.PackSizeHigh = pi.PackSizeHigh;
    pw.PackSize = pi.PackSize;
    pw.Flags = pi.Flags;
    pw.NumberOfLinks = pi.NumberOfLinks;

    pw.Description = a2w(pi.Description, CP_OEMCP);
    pw.Owner = a2w(pi.Owner, CP_OEMCP);

    pw.UserData = pi.UserData;
    pw.CRC32 = pi.CRC32;
    pw.Reserved[0] = pi.Reserved[0];
    pw.Reserved[1] = pi.Reserved[1];

    pw.CustomColumnData.size(pi.CustomColumnNumber);

    for (int i = 0; i < pi.CustomColumnNumber; i++)
    {
        pw.CustomColumnData[i] = a2w(pi.CustomColumnData[i], CP_OEMCP);
    }

    return pw;
}

static PanelInfoW& copy(PanelInfoW& pw, const PanelInfo& pi)
{
    pw.PanelType = pi.PanelType;
    pw.Plugin = pi.Plugin;
    pw.PanelRect = pi.PanelRect;

    pw.CurrentItem = pi.CurrentItem;
    pw.TopPanelItem = pi.TopPanelItem;
    pw.Visible = pi.Visible;
    pw.Focus = pi.Focus;
    pw.ViewMode = pi.ViewMode;

    pw.ShortNames = pi.ShortNames;
    pw.SortMode = pi.SortMode;

    pw.Flags = pi.Flags;
    pw.Reserved = pi.Reserved;

    return pw;
}

bool FarGetActivePanelInfo(PanelInfoW& pw)
{
    PanelInfo pi;
    if (!FarGetPanelInfo(&pi, true, false))
        return false;

    copy(pw, pi);

    return true;
}

bool FarGetPassivePanelInfo(PanelInfoW& pw)
{
    PanelInfo pi;
    if (!FarGetPanelInfo(&pi, false, false))
        return false;

    copy(pw, pi);

    return true;
}

MyStringW FarGetActivePanelDirectory()
{
    MyStringW res;

    PanelInfo pi;

    if (FarGetPanelInfo(&pi, true, false))
    {
        res = a2w(pi.CurDir, CP_OEMCP);
    }

    return res;
}

MyStringW FarGetPassivePanelDirectory()
{
    MyStringW res;

    PanelInfo pi;

    if (FarGetPanelInfo(&pi, false, false))
    {
        res = a2w(pi.CurDir, CP_OEMCP);
    }

    return res;
}

static PluginPanelItemsW FarGetPanelItems(bool active, bool selected)
{
    PluginPanelItemsW res;

    PanelInfo pi;
    if (FarGetPanelInfo(&pi, active, true))
    {
        int count;
        PluginPanelItem* items;

        if (selected)
        {
            count = pi.SelectedItemsNumber;

            if (count == 1)
            {
                items = &pi.PanelItems[pi.CurrentItem];
            }
            else
            {
                items = pi.SelectedItems;
            }
        }
        else
        {
            count = pi.ItemsNumber;
            items = pi.PanelItems;
        }

        res.size(count);

        for (int i = 0; i < count; i++)
        {
            copy(res[i], items[i]);
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

        hwnd = (HWND)theFar.AdvControl(theFar.ModuleNumber, ACTL_GETFARHWND, NULL);

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

/**
 * Writes data to the registry
 */
bool FarWriteRegistry(const wchar_t* name, DWORD type, const void* value, size_t size)
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
size_t FarReadRegistry(const wchar_t* name, DWORD type, void* value, size_t size)
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

/**
 * Truncates a string.
 */
MyStringA TruncPathStr(const MyStringA& s, int maxLen)
{
    if (!theFar.FSF->TruncPathStr)
        return MyStringA(s);

    MyStringA res(s);

    return res = theFar.FSF->TruncPathStr(res, maxLen);
}

MyStringW TruncPathStr(const MyStringW& s, int maxLen)
{
    MyStringA sA = w2a(s, CP_OEMCP);

    return a2w(TruncPathStr(sA, maxLen), CP_OEMCP);
}

// vim: set et ts=4 ai :

