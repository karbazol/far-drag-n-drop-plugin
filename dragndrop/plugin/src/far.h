/** @file far.h
 * The file contains declarations for Far API.
 *
 * $Id: far.h 81 2011-11-07 08:50:02Z Karbazol $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__FAR_H__
#define __KARBAZOL_DRAGNDROP_2_0__FAR_H__

#pragma warning(push, 3)
#include "plugin.hpp"
#pragma warning(pop)
#include <dll/mystring.h>
#include <common/growarry.h>

struct PluginPanelItemW
{
    FILETIME CreationTime;
    FILETIME LastAccessTime;
    FILETIME LastWriteTime;
    FILETIME ChangeTime;

    unsigned __int64 FileSize;
    unsigned __int64 AllocationSize;
    
    MyStringW       FileName;
    MyStringW       AlternateFileName;
    MyStringW       Description;
    MyStringW       Owner;
    GrowOnlyArray<MyStringW> CustomColumnData;
    PLUGINPANELITEMFLAGS Flags;
    struct
    {
        void *Data;
        FARPANELITEMFREECALLBACK FreeData;
    } UserData;
    uintptr_t FileAttributes;
    uintptr_t NumberOfLinks;
    uintptr_t CRC32;
    intptr_t Reserved[2];

    PluginPanelItemW():  FileSize(0), AllocationSize(0),
        FileName(), AlternateFileName(), Description(), Owner(),
        CustomColumnData(), Flags(0), FileAttributes(0), NumberOfLinks(0),
        CRC32(0)
    {
        CreationTime.dwLowDateTime = 0;
        CreationTime.dwHighDateTime = 0;
        LastAccessTime.dwLowDateTime = 0;
        LastAccessTime.dwHighDateTime = 0;
        LastWriteTime.dwLowDateTime = 0;
        LastWriteTime.dwHighDateTime = 0;
        ChangeTime.dwLowDateTime = 0;
        ChangeTime.dwHighDateTime = 0;

        UserData.Data = 0;
        UserData.FreeData = 0;
        Reserved[0] = 0;
        Reserved[1] = 0;
    }
};

typedef GrowOnlyArray<PluginPanelItemW> PluginPanelItemsW;

// Standard Far Functions

const wchar_t* GetMsg(int MsgId);

#ifndef FAR_CONSOLE_GET_MODE
#define FAR_CONSOLE_GET_MODE       (-2)
#endif
#ifndef FAR_CONSOLE_TRIGGER
#define FAR_CONSOLE_TRIGGER        (-1)
#endif
#ifndef FAR_CONSOLE_SET_WINDOWED
#define FAR_CONSOLE_SET_WINDOWED   (0)
#endif
#ifndef FAR_CONSOLE_SET_FULLSCREEN
#define FAR_CONSOLE_SET_FULLSCREEN (1)
#endif
#ifndef FAR_CONSOLE_WINDOWED
#define FAR_CONSOLE_WINDOWED       (0)
#endif
#ifndef FAR_CONSOLE_FULLSCREEN
#define FAR_CONSOLE_FULLSCREEN     (1)
#endif

/**
 * Returns TRUE if the far.exe is running in console mode otherwise FALSE
 */
int ConsoleMode(int param);
bool FarGetWindowInfo(WindowInfo& wip);
bool FarGetActivePanelInfo(PanelInfo& p);
bool FarGetPassivePanelInfo(PanelInfo& p);
MyStringW FarGetActivePanelDirectory();
MyStringW FarGetPassivePanelDirectory();
PluginPanelItemsW FarGetActivePanelItems(bool selected);
PluginPanelItemsW FarGetPassivePanelItems(bool selected);
HWND GetFarWindow();
MyStringA TruncPathStr(const MyStringA& s, int maxLen);
MyStringW TruncPathStr(const MyStringW& s, int maxLen);


extern BOOL (*IsActiveFar)();

// Registry access functions
bool FarWriteRegistry(const wchar_t* name, const DWORD value);
DWORD FarReadRegistry(const wchar_t* name, DWORD defaultValue=0);

#endif // __KARBAZOL_DRAGNDROP_2_0__FAR_H__
// vim: set et ts=4 ai :

