#ifndef __KARBAZOL_DRAGNDROP_2_0__DNDCMNCT_H__
#define __KARBAZOL_DRAGNDROP_2_0__DNDCMNCT_H__

#include <windows.h>
#include "dndmsgs.h"
#include <common/hldrdefs.h>

inline void showDndWindow(HWND hwnd){SendMessage(hwnd, WM_SHOWDNDWINDOW, 0, 0);}
inline void hideDndWindow(HWND hwnd){SendMessage(hwnd, WM_HIDEDNDWINDOW, 0, 0);}
inline bool isDndDropTarget(HWND hwnd){return SendMessage(hwnd, WM_ISDRAGGIN, 0, 0)==HOLDER_YES?true:false;}
inline bool isDndWindow(HWND hwnd){return SendMessage(hwnd, WM_HLDR_ISDNDWND, 0, 0)==HOLDER_YES?true:false;}

#endif // __KARBAZOL_DRAGNDROP_2_0__DNDCMNCT_H__

