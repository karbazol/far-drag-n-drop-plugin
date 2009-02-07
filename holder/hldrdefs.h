#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRDEFS_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRDEFS_H__

#include <windows.h>

// Holder window class name
#define HOLDER_CLASS_NAME L"Karbazol_dragndrop_hook_holder_window"
#define HOLDER_MUTEX L"Karbazol_dragndrop_hook_holder_mutex"
#define HOLDER_LEFT_EVENT L"Karbazol_dragndrop_hook_holder_left_event"
#define HOLDER_RIGHT_EVENT L"Karbazol_dragndrop_hook_holder_right_event"

// Holder window will answer either

//HYES
#define HOLDER_YES 0x53455948

//HNOT
#define HOLDER_NOT 0x544f4e48

// Holder window message identifiers

#define WM_HOLDERBASE WM_USER + 1998

/**
 * Notify holder about far windows created
 * wParam - hwnd of Far window
 * lParam - hwnd of DND window
 */
#define WM_FARWINDOWSCREATED WM_HOLDERBASE + 1

/**
 * Far DND window is about to destroy
 * wParam - hwnd of DND window
 */
#define WM_DNDWINDOWDESTROY WM_HOLDERBASE + 2

/**
 * Check whether specified hwnd is Far
 * wParam - hwnd to check
 */
#define WM_HLDR_ISFARWND WM_HOLDERBASE + 3

/**
 * Check whether specified hwnd is DND
 * wParam - hwnd to check
 */
#define WM_HLDR_ISDNDWND WM_HOLDERBASE + 4

/**
 * Check whether specified hwnd is Far
 * wParam - dnd hwnd
 * result - far hwnd or HNOT
 */
#define WM_HLDR_GETFARWND WM_HOLDERBASE + 5

/**
 * Check whether specified hwnd is Far
 * wParam - far hwnd
 * result - dnd hwnd or HNOT
 */
#define WM_HLDR_GETDNDWND WM_HOLDERBASE + 6

/**
 * Are you holder?
 * Answer HYES
 */

#define WM_ARE_YOU_HOLDER WM_HOLDERBASE + 7

/**
 * wParam == TRUE - set the hook
 * wParam == FALSE - reset the hook
 */
#define WM_HLDR_SETHOOK WM_HOLDERBASE + 8

#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRDEFS_H__

