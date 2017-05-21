#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRDEFS_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRDEFS_H__

#include <windows.h>

#if defined(TARGET_ARCH_x86)
#   ifndef TARGET_ARCH
#       define TARGET_ARCH L"x86"
#   endif
#   ifndef OTHER_ARCH
#       define OTHER_ARCH L"x86_64"
#   endif
#elif defined(TARGET_ARCH_x86_64)
#   ifndef TARGET_ARCH
#       define TARGET_ARCH L"x86_64"
#   endif
#   ifndef OTHER_ARCH
#       define OTHER_ARCH L"x86"
#   endif
#else
#   error "Either TARGET_ARCH_x86 or TARGET_ARCH_x86_64 must be defined"
#endif

// Holder window class name
#define HOLDER_CLASS_NAME L"Karbazol_dragndrop_hook_holder_window" TARGET_ARCH
#define HOLDER_OTHER_CLASS_NAME L"Karbazol_dragndrop_hook_holder_window" OTHER_ARCH
#define HOLDER_MUTEX L"Karbazol_dragndrop_hook_holder_mutex" TARGET_ARCH
#define HOLDER_OTHER_MUTEX L"Karbazol_dragndrop_hook_holder_mutex" OTHER_ARCH
#define HOLDER_LEFT_EVENT L"Karbazol_dragndrop_hook_holder_left_event" TARGET_ARCH
#define HOLDER_OTHER_LEFT_EVENT L"Karbazol_dragndrop_hook_holder_left_event" OTHER_ARCH
#define HOLDER_RIGHT_EVENT L"Karbazol_dragndrop_hook_holder_right_event" TARGET_ARCH
#define HOLDER_OTHER_RIGHT_EVENT L"Karbazol_dragndrop_hook_holder_right_event" OTHER_ARCH
#define HOLDER_EXECUTABLE L"holder_" TARGET_ARCH L".exe"
#define HOLDER_OTHER_EXECUTABLE L"holder_" OTHER_ARCH L".exe"

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
/* Forwarding to other arch holder */
#define WM_FARWINDOWSCREATED_FWD WM_HOLDERBASE + 2

/**
 * Far DND window is about to destroy
 * wParam - hwnd of DND window
 */
#define WM_DNDWINDOWDESTROY WM_HOLDERBASE + 3
/* Forwarding to other arch holder */
#define WM_DNDWINDOWDESTROY_FWD WM_HOLDERBASE + 4

/**
 * Check whether specified hwnd is Far
 * wParam - hwnd to check
 */
#define WM_HLDR_ISFARWND WM_HOLDERBASE + 5

/**
 * Check whether specified hwnd is DND
 * wParam - hwnd to check
 */
#define WM_HLDR_ISDNDWND WM_HOLDERBASE + 7

/**
 * Check whether specified hwnd is Far
 * wParam - dnd hwnd
 * result - far hwnd or HNOT
 */
#define WM_HLDR_GETFARWND WM_HOLDERBASE + 9

/**
 * Check whether specified hwnd is Far
 * wParam - far hwnd
 * result - dnd hwnd or HNOT
 */
#define WM_HLDR_GETDNDWND WM_HOLDERBASE + 11

/**
 * Are you holder?
 * Answer HYES
 */

#define WM_ARE_YOU_HOLDER WM_HOLDERBASE + 13

/**
 * wParam == TRUE - set the hook
 * wParam == FALSE - reset the hook
 */
#define WM_HLDR_SETHOOK WM_HOLDERBASE + 15

#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRDEFS_H__

