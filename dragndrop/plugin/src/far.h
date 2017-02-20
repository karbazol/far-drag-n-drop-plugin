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
#include <mystring.h>
#include <growarry.h>

struct PluginPanelItemW
{
    MyStringW cFileName;
};

typedef GrowOnlyArray<PluginPanelItemW> PluginPanelItemsW;

struct PanelInfoW
{
    int Plugin;
    RECT PanelRect;
    int Visible;
    DWORD Flags;
};

struct WindowInfoW
{
  intptr_t Pos;
  int Type;
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

#ifdef FAR2
typedef int FAR_WPARAM_TYPE;
typedef LONG_PTR FAR_LPARAM_TYPE;
typedef LONG_PTR FAR_RETURN_TYPE;
#else
typedef intptr_t FAR_WPARAM_TYPE;
typedef void* FAR_LPARAM_TYPE;
typedef intptr_t FAR_RETURN_TYPE;
#endif

#endif // __KARBAZOL_DRAGNDROP_2_0__FAR_H__
// vim: set et ts=4 ai :

