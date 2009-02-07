/** @file far.h
 * The file contains declarations for Far API.
 *
 * $Id: far.h 26 2008-04-20 18:48:32Z eleskine $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__FAR_H__
#define __KARBAZOL_DRAGNDROP_2_0__FAR_H__

#pragma warning(push, 3)
#include "plugin.hpp"
#pragma warning(pop)

// Standard Far Functions

const char* GetMsg(int MsgId);
long WINAPI DefDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2);

int DialogEx(int X1, int Y1, int X2, int Y2,
  const char *HelpTopic, struct FarDialogItem *Item, int ItemsNumber,
  DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc=&DefDlgProc, long Param=NULL);
long SendDlgMessage(HANDLE hDlg, int Msg, int Param1, long Param2);

struct InitDialogItem
{
    int Type;
    int X1;
    int Y1;
    int X2;
    int Y2;
    int Focus;
    int Selected;
    unsigned int Flags;
    int DefaultButton;
    char *Data;
};

void InitDialogItems(
       const struct InitDialogItem *Init,
       struct FarDialogItem *Item,
       int ItemsNumber
);

/**
 * Returns TRUE if the far.exe is running in console mode otherwise FALSE
 */
int ConsoleMode(int param);
bool FarGetWindowInfo(struct WindowInfo* wi);
bool FarGetPanelInfo(struct PanelInfo* pi);
bool FarGetShortPanelInfo(struct PanelInfo* pi);
bool FarGetShortOtherPanelInfo(struct PanelInfo* pi);
int AddEndSlashA(char* path);
HWND GetFarWindow();
char* TruncPathStr(char* s, int maxLen);


BSTR PanelItemToWidePath(const char* path, struct PluginPanelItem& pi, LPWSTR* filePart=0);

extern BOOL (*IsActiveFar)();

// Registry access functions
bool FarWriteRegistry(const char* name, const DWORD value);
DWORD FarReadRegistry(const char* name, DWORD default=0);

#endif // __KARBAZOL_DRAGNDROP_2_0__FAR_H__
// vim: set et ts=4 ai :

