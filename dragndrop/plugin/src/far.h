/** @file far.h
 * The file contains declarations for Far API.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__FAR_H__
#define __KARBAZOL_DRAGNDROP_2_0__FAR_H__

#pragma warning(push, 3)
#include "plugin.hpp"
#pragma warning(pop)
#include "mystring.h"
#include "growarry.h"

struct FAR_FIND_DATA_W
{
    DWORD     dwFileAttributes;
    FILETIME  ftCreationTime;
    FILETIME  ftLastAccessTime;
    FILETIME  ftLastWriteTime;
    DWORD     nFileSizeHigh;
    DWORD     nFileSizeLow;
    DWORD     dwReserved0;
    DWORD     dwReserved1;
    MyStringW cFileName;
    MyStringW cAlternateFileName;
    FAR_FIND_DATA_W(): cFileName(), cAlternateFileName(){}
};

struct PluginPanelItemW
{
    FAR_FIND_DATA_W FindData;
    DWORD           PackSizeHigh;
    DWORD           PackSize;
    DWORD           Flags;
    DWORD           NumberOfLinks;
    MyStringW       Description;
    MyStringW       Owner;
    GrowOnlyArray<MyStringW> CustomColumnData;
    DWORD_PTR       UserData;
    DWORD           CRC32;
    DWORD_PTR       Reserved[2];
    PluginPanelItemW(): Description(), Owner(), CustomColumnData(){}
};

typedef GrowOnlyArray<PluginPanelItemW> PluginPanelItemsW;

struct PanelInfoW
{
    int PanelType;
    int Plugin;
    RECT PanelRect;
    int CurrentItem;
    int TopPanelItem;
    int Visible;
    int Focus;
    int ViewMode;
    int ShortNames;
    int SortMode;
    DWORD Flags;
    DWORD Reserved;
    PanelInfoW(){}
};

struct WindowInfoW
{
  int  Pos;
  int  Type;
  int  Modified;
  int  Current;
  MyStringW TypeName;
  MyStringW Name;

  WindowInfoW(): Pos(0), Type(0), Modified(0), Current(0), TypeName(), Name(){}
};

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
bool FarGetWindowInfo(WindowInfoW& wip);
bool FarGetActivePanelInfo(PanelInfoW& p);
bool FarGetPassivePanelInfo(PanelInfoW& p);
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

