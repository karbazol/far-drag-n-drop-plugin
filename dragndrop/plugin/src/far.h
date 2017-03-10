/** @file far.h
 * The file contains declarations for Far API.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__FAR_H__
#define __KARBAZOL_DRAGNDROP_2_0__FAR_H__

#pragma warning(push, 3)
#include "plugin.hpp"
#pragma warning(pop)
#include <mystring.h>
#include <growarry.h>

struct PluginPanelItemW
{
    FILETIME CreationTime;
    FILETIME LastAccessTime;
    FILETIME LastWriteTime;
    FILETIME ChangeTime;

    uint64_t FileSize;
    uint64_t AllocationSize;
    
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

bool FarGetWindowInfo(WindowInfo& wip);
bool FarGetActivePanelInfo(PanelInfo& p);
bool FarGetPassivePanelInfo(PanelInfo& p);
MyStringW FarGetActivePanelDirectory();
MyStringW FarGetPassivePanelDirectory();
PluginPanelItemsW FarGetActivePanelItems(bool selected);
PluginPanelItemsW FarGetPassivePanelItems(bool selected);
bool FarClearSelectionActivePanel(intptr_t index, intptr_t count);
HWND GetFarWindow();
MyStringA TruncPathStr(const MyStringA& s, int maxLen);
MyStringW TruncPathStr(const MyStringW& s, int maxLen);


extern BOOL (*IsActiveFar)();

// Registry access functions
bool FarWriteRegistry(const wchar_t* name, const DWORD value);
DWORD FarReadRegistry(const wchar_t* name, DWORD defaultValue=0);

#endif // __KARBAZOL_DRAGNDROP_2_0__FAR_H__
// vim: set et ts=4 ai :

