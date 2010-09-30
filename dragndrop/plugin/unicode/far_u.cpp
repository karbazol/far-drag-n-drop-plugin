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
int WINAPI GetMinFarVersionW()
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
        IsActiveFar = &alwaysTrue;
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
        regKeyName = new MyStringW(theFar.RootKey);
        *regKeyName /= L"Karbazol\\DragNDrop";

        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&freeRegistry), regKeyName);
    }

    return *regKeyName;
}

/**
 * Far calls this function to provide its API to the plug-in.
 * The plugin here performs its initialization.
 */
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{                     
    checkConman();

    if (!patchImports())
        return;

    // Create MainThread instance
    MainThread::instance();
    
    TRACE("Setting plugin startup info\n");
    theFar = *Info;
    farFuncs = *Info->FSF;
    theFar.FSF = &farFuncs;

    // Make sure we have correct registry key name
    getRegistryKeyName();

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
    Info->PluginConfigStrings = ConfigStrings;
    Info->PluginConfigStringsNumber = LENGTH(ConfigStrings);
}

int WINAPI ConfigureW(int Number)
{
    return doConfigure(Number);
}
/**
 * Far's default dialog procedure
 */
long WINAPI DefDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
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
HANDLE DialogInit(int X1, int Y1, int X2, int Y2,
  const wchar_t *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, long Param)
{
    HANDLE theDialog = INVALID_HANDLE_VALUE;
    if (theFar.DialogInit)
    {
        theDialog = theFar.DialogInit(theFar.ModuleNumber, X1, Y1, X2, Y2,
                HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
        return theDialog;
    }

    return INVALID_HANDLE_VALUE;
}

/**
 * Far's function to run a dialog
 */
int DialogRun(HANDLE dlg)
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
        return theFar.GetMsg(theFar.ModuleNumber, MsgId);
    }

    return 0;
}

/**
 * Sends a message to specified dialog
 */
long SendDlgMessage(HANDLE hDlg, int Msg, int Param1, long Param2)
{
    if (theFar.SendDlgMessage)
        return theFar.SendDlgMessage(hDlg, Msg, Param1, Param2);

    return -1;
}

/**
 * Helps to determine wheather the Far is running in Full-screen mode or 
 * in windowed mode
 */
int ConsoleMode(int param)
{
    if (theFar.AdvControl)
        return theFar.AdvControl(theFar.ModuleNumber, ACTL_CONSOLEMODE, (void*)param);
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

    wip.TypeName = wi.TypeName;
    wip.Name = wi.Name;

    return wip;
}

bool FarGetWindowInfo(WindowInfoW& wip)
{
    struct WindowInfo wi;
    wi.Pos = wip.Pos;
    wi.TypeNameSize = 0;
    wi.NameSize = 0;
    if (!FarGetWindowInfo(&wi))
        return false;

    wip.TypeName.length(wi.TypeNameSize);
    wi.TypeName = wip.TypeName;

    wip.Name.length(wi.NameSize);
    wi.Name = wip.Name;

    if (!FarGetWindowInfo(&wi))
        return false;

    copy(wip, wi);
    return true;
}
/**
 * Gets Panel info from the Far.
 */
bool FarGetPanelInfo(struct PanelInfo* pi)
{
    if (theFar.Control)
        return theFar.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, reinterpret_cast<LONG_PTR>(pi))?true:false;
    return false;
}

static FAR_FIND_DATA_W& copy(FAR_FIND_DATA_W& pw, const FAR_FIND_DATA& pi)
{
    pw.dwFileAttributes = pi.dwFileAttributes;
    pw.ftCreationTime = pi.ftCreationTime;
    pw.ftLastAccessTime = pi.ftLastAccessTime;
    pw.ftLastWriteTime = pi.ftLastWriteTime;
    pw.nFileSizeHigh = ((LARGE_INTEGER*)&pi.nFileSize)->HighPart;
    pw.nFileSizeLow = ((LARGE_INTEGER*)&pi.nFileSize)->LowPart;
    //pw.dwReserved0 = pi.dwReserved0;
    //pw.dwReserved1 = pi.dwReserved1;

    pw.cFileName = pi.lpwszFileName;
    pw.cAlternateFileName = pi.lpwszAlternateFileName;

    return pw;
}

static PluginPanelItemW& copy(PluginPanelItemW& pw, const PluginPanelItem& pi)
{
    copy(pw.FindData, pi.FindData);

#if 0
    pw.PackSizeHigh = pi.PackSizeHigh;
    pw.PackSize = pi.PackSize;
#endif
    pw.Flags = pi.Flags;
    pw.NumberOfLinks = pi.NumberOfLinks;

    pw.Description = pi.Description;
    pw.Owner = pi.Owner;

    pw.UserData = pi.UserData;
    pw.CRC32 = pi.CRC32;
    pw.Reserved[0] = pi.Reserved[0];
    pw.Reserved[1] = pi.Reserved[1];

    pw.CustomColumnData.size(pi.CustomColumnNumber);

    for (int i = 0; i < pi.CustomColumnNumber; i++)
    {
        pw.CustomColumnData[i] = pi.CustomColumnData[i];
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

    //pw.ColumnTypes = pi.lpwszColumnTypes;
    //pw.ColumnWidths = pi.lpwszColumnWidths;
    //pw.CurDir = pi.lpwszCurDir;

    pw.ShortNames = pi.ShortNames;
    pw.SortMode = pi.SortMode;

    pw.Flags = pi.Flags;
    pw.Reserved = pi.Reserved;

#if 0
    pw.PanelItems.size(0);
    pw.SelectedItems.size(0);

    if (pi.PanelItems)
    {
        pw.PanelItems.size(pi.ItemsNumber);

        int i;

        for (i = 0; i < pi.ItemsNumber; i++)
        {
            PluginPanelItemW& item = copy(pw.PanelItems[i], pi.PanelItems[i]);
            if (pi.SelectedItemsNumber && (item.Flags & PPIF_SELECTED))
                pw.SelectedItems.append(&item);
        }

        if (pi.SelectedItemsNumber == 1 && !pw.SelectedItems.size())
        {
            pw.SelectedItems.append(&pw.PanelItems[pw.CurrentItem]);
        }

        ASSERT(pw.SelectedItems.size() == (size_t)pi.SelectedItemsNumber);
    }
#endif

    return pw;
}

bool FarGetActivePanelInfo(PanelInfoW& pw)
{
    PanelInfo pi;
    if (!FarGetPanelInfo(&pi))
        return false;

    copy(pw, pi);

    return true;
}

static bool FarGetPassivePanelInfo(struct PanelInfo* pi)
{
    if (theFar.Control)
        return theFar.Control(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, reinterpret_cast<LONG_PTR>(pi))?true:false;
    return false;
}

bool FarGetPassivePanelInfo(PanelInfoW& p)
{
    PanelInfo pi;
    if (!FarGetPassivePanelInfo(&pi))
        return false;

    copy(p, pi);

    return true;
}

static MyStringW FarGetPanelDirectory(bool activePanel)
{
    MyStringW res;

    if (theFar.Control)
    {
        HANDLE h = activePanel?PANEL_ACTIVE:PANEL_PASSIVE;

        size_t buffSize = theFar.Control(h, FCTL_GETCURRENTDIRECTORY, 0, 0);

        wchar_t* buff = new wchar_t[buffSize];

        theFar.Control(h, FCTL_GETCURRENTDIRECTORY, buffSize,
                reinterpret_cast<LONG_PTR>(static_cast<wchar_t*>(buff)));

        res = buff;

        delete [] buff;
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

PluginPanelItemsW FarGetPanelItems(bool active, bool selected)
{
    PluginPanelItemsW res;

    if (theFar.Control)
    {
        HANDLE h = active?PANEL_ACTIVE:PANEL_PASSIVE;
        int command = selected?FCTL_GETSELECTEDPANELITEM:FCTL_GETPANELITEM;

        PanelInfo pi;

        if (theFar.Control(h, FCTL_GETPANELINFO, 0, reinterpret_cast<LONG_PTR>(&pi)))
        {
            int count = selected?pi.SelectedItemsNumber:pi.ItemsNumber;
            int i;

            res.size(count);

            if (count > 1)
            {

                for (i = 0; i < count; ++i)
                {
                    PluginPanelItem item;
                    theFar.Control(h, command, i, reinterpret_cast<LONG_PTR>(&item));

                    copy(res[i], item);

                    //theFar.Control(h, FCTL_FREEPANELITEM, 0, reinterpret_cast<LONG_PTR>(&item));
                }
            }
            else
            {
                PluginPanelItem item;
                theFar.Control(h, FCTL_GETPANELITEM, pi.CurrentItem, reinterpret_cast<LONG_PTR>(&item));

                copy(res[0], item);

                //theFar.Control(h, FCTL_FREEPANELITEM, 0, reinterpret_cast<LONG_PTR>(&item));
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
                reinterpret_cast<const BYTE*>(value), size));

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

    DWORD res = size;

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

// vim: set et ts=4 ai :
