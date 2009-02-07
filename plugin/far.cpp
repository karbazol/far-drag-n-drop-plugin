/**
 * @file far.cpp
 * The file contains implementation of Far API.
 *
 * $Id: far.cpp 26 2008-04-20 18:48:32Z eleskine $
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

static struct PluginStartupInfo theFar={0};
static struct FarStandardFunctions farFuncs={0};

int WINAPI GetMinFarVersion()
{
    return FARMANAGERVERSION;
}

static BOOL alwaysTrue()
{
    return TRUE;
}

BOOL (*IsActiveFar)() = 0;

static void checkConman()
{
    HMODULE conman = GetModuleHandle(L"infis.dll");
    if (conman)
    {
        IsActiveFar = (BOOL (*)())GetProcAddress(conman, "IsConsoleActive");
    }
    if (!IsActiveFar)
        IsActiveFar = alwaysTrue;
}

static void freeRegistry(char* registry)
{
    delete [] registry;
}

static const char* getRegistryKeyName()
{
    static char* regKeyName = 0;
    if (!regKeyName)
    {
        static const char* name = "Karbazol\\DragNDrop";
        size_t size = lstrlenA(theFar.RootKey) + 2 + lstrlenA(name);

        regKeyName = new char[size];

        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&freeRegistry), regKeyName);

        lstrcpyA(regKeyName, theFar.RootKey);
        theFar.FSF->AddEndSlash(regKeyName);

        lstrcatA(regKeyName, name);
    }

    return regKeyName;
}

void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
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

void WINAPI ExitFAR(void)
{
    ThreadPool::instance()->shutDown();
    Dragging::instance()->shutDown();
    restoreImports();
}

void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
    Info->StructSize = sizeof(*Info);
    Info->Flags = PF_PRELOAD;

    static const char* ConfigStrings[1];
    ConfigStrings[0] = GetMsg(MConfigMenu);
    Info->PluginConfigStrings = ConfigStrings;
    Info->PluginConfigStringsNumber = LENGTH(ConfigStrings);
}

void InitDialogItems(
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
            lstrcpynA(PItem->Data,GetMsg((int)PInit->Data), LENGTH(PItem->Data));
        else
            lstrcpynA(PItem->Data,PInit->Data,LENGTH(PItem->Data));
    }
}

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

const char* GetMsg(int MsgId)
{
    if (theFar.GetMsg)
    {
        return theFar.GetMsg(theFar.ModuleNumber, MsgId);
    }

    return 0;
}

long SendDlgMessage(HANDLE hDlg, int Msg, int Param1, long Param2)
{
    if (theFar.SendDlgMessage)
        return theFar.SendDlgMessage(hDlg, Msg, Param1, Param2);

    return -1;
}

int ConsoleMode(int param)
{
    if (theFar.AdvControl)
        return theFar.AdvControl(theFar.ModuleNumber, ACTL_CONSOLEMODE, (void*)param);
    return -1;
}

bool FarGetWindowInfo(struct WindowInfo* wi)
{
    if (theFar.AdvControl)
        return theFar.AdvControl(theFar.ModuleNumber, ACTL_GETWINDOWINFO, wi)?true:false;

    return false;
}

bool FarGetPanelInfo(struct PanelInfo* pi)
{
    if (theFar.Control)
        return theFar.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, pi)?true:false;
    return false;
}

bool FarGetShortPanelInfo(struct PanelInfo* pi)
{
    if (theFar.Control)
        return theFar.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELSHORTINFO, pi)?true:false;
    return false;
}

bool FarGetShortOtherPanelInfo(struct PanelInfo* pi)
{
    if (theFar.Control)
        return theFar.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELSHORTINFO, pi)?true:false;
    return false;
}

int AddEndSlashA(char* path)
{
    if (theFar.FSF && theFar.FSF->AddEndSlash)
        return theFar.FSF->AddEndSlash(path);
    return 0;
}

BSTR PanelItemToWidePath(const char* path, struct PluginPanelItem& pi, LPWSTR* filePart)
{
    if (!path || !*path)
        return 0;

    UINT cp = AreFileApisANSI()?CP_ACP:CP_OEMCP;

    int strLen = MultiByteToWideChar(cp, 0, path, -1, 0, 0);
    if (!strLen)
        return 0;

    LPWSTR res = SysAllocStringLen(NULL, strLen+2+MAX_PATH);
    if (!res)
        return 0;

    if (!MultiByteToWideChar(cp, 0, path, -1, res, strLen+2))
    {
        SysFreeString(res);
        return 0;
    }

    LPWSTR slash;

    for (slash = StrChrW(res, L'/'); slash; slash = StrChrW(slash, L'/'))
    {
        *slash = L'\\';
    }

    LPWSTR fileName = res+lstrlenW(res);
    if (fileName[-1] != L'\\')
        *fileName++ = L'\\';

    if (pi.FindData.cAlternateFileName[0] && pi.FindData.cAlternateFileName[12] == '\0')
        MultiByteToWideChar(cp, 0, pi.FindData.cAlternateFileName, -1, fileName, MAX_PATH);
    else
        MultiByteToWideChar(cp, 0, pi.FindData.cFileName, -1, fileName, MAX_PATH);

    // REMOVESTATIC
    WIN32_FIND_DATAW fd;
    HANDLE hData = FindFirstFileW(res, &fd);
    if (hData == INVALID_HANDLE_VALUE)
    {
        SysFreeString(res);
        return 0;
    }

    *fileName = 0;

    do
    {
        if (fd.dwFileAttributes == pi.FindData.dwFileAttributes&&
            fd.ftCreationTime.dwLowDateTime == pi.FindData.ftCreationTime.dwLowDateTime&&
            fd.ftCreationTime.dwHighDateTime == pi.FindData.ftCreationTime.dwHighDateTime&&
            fd.ftLastAccessTime.dwLowDateTime == pi.FindData.ftLastAccessTime.dwLowDateTime&&
            fd.ftLastAccessTime.dwHighDateTime == pi.FindData.ftLastAccessTime.dwHighDateTime&&
            fd.ftLastWriteTime.dwLowDateTime == pi.FindData.ftLastWriteTime.dwLowDateTime&&
            fd.ftLastWriteTime.dwHighDateTime == pi.FindData.ftLastWriteTime.dwHighDateTime&&
            fd.nFileSizeHigh == pi.FindData.nFileSizeHigh&&
            fd.nFileSizeLow == pi.FindData.nFileSizeLow&&
            fd.dwReserved0 == pi.FindData.dwReserved0&&
            fd.dwReserved1 == pi.FindData.dwReserved1)
        {
            lstrcpyW(fileName, fd.cFileName);
            break;
        }
    } while (FindNextFileW(hData, &fd));

    FindClose(hData);

    if (!*fileName)
    {
        SysFreeString(res);
        return 0;
    }

    if (filePart)
        *filePart = fileName;

    return res;
}

HWND GetFarWindow()
{
    static HWND hwnd = 0;
    if (!hwnd)
    {
        if (!theFar.AdvControl)
            return NULL;

        hwnd = (HWND)theFar.AdvControl(theFar.ModuleNumber, ACTL_GETFARHWND, NULL);
        
#if 0
        // @TODO Investigate case of ConEmu
        HWND parent = GetAncestor(hwnd, GA_PARENT);
        if (parent != (HWND)0x10010)
        {
            // This mean far is run in ConEmu not in generic console
            hwnd = parent;
        }
#endif
    }

    return hwnd;
}

bool FarWriteRegistry(const char* name, DWORD type, const void* value, size_t size)
{
    HKEY key = 0;
    
    if (ERROR_SUCCESS != RegCreateKeyExA(HKEY_CURRENT_USER, getRegistryKeyName(), 0,
            NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL))
        return false;

    bool res = (ERROR_SUCCESS == RegSetValueExA(key, name, 0, type, 
                reinterpret_cast<const BYTE*>(value), size));

    RegCloseKey(key);

    return res;
}

bool FarWriteRegistry(const char* name, const DWORD value)
{
    return FarWriteRegistry(name, REG_DWORD, (const BYTE*)&value, sizeof(value));
}

size_t FarReadRegistry(const char* name, DWORD type, void* value, size_t size)
{
    HKEY key = 0;

    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_CURRENT_USER, getRegistryKeyName(),
                0, KEY_READ, &key))
        return 0;

    DWORD res = size;

    if (ERROR_SUCCESS != RegQueryValueExA(key, name, NULL, &type, (BYTE*)value, &res))
        res = 0;

    RegCloseKey(key);

    return res;
}

DWORD FarReadRegistry(const char* name, DWORD default)
{
    DWORD res = default;

    FarReadRegistry(name, REG_DWORD, &res, sizeof(res));

    return res;
}

char* TruncPathStr(char* s, int maxLen)
{
    if (!theFar.FSF->TruncPathStr)
        return s;

    return theFar.FSF->TruncPathStr(s, maxLen);
}

// vim: set et ts=4 ai :

