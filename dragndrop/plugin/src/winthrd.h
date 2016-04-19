/**
 * @file winthrd.h
 * Contains declaration of WinThread class
 *
 * $Id: winthrd.h 77 2011-10-22 20:25:46Z Karbazol $
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__WINTHRD_H__
#define __KARBAZOL_DRAGNDROP_2_0__WINTHRD_H__

#include <windows.h>
#include <dll/dndmsgs.h>
#include <hook/hldrapi.h>

#include "toolwnd.h"
#include "datacont.h"

/**
 * @brief Window thread singleton.
 *
 * Singleton. Represents the window thread. This thread performs dragging and dropping.
 */
class WinThread: public IHolder
{
private:
    HANDLE _handle;
    HANDLE _leftButtonEvent;
    HANDLE _rightButtonEvent;
    DWORD _id;
    ToolWindow _window;
    WinThread();
    WinThread(const WinThread&);
    ~WinThread();
    static DWORD WINAPI runInstnace(WinThread* p);
    static void kill(WinThread* p);
    void run();
private:
    // IHolder implementation
    const wchar_t* getHolderWindowClassName();
    const wchar_t* getHolderFileName();
    const wchar_t* getHolderMutexName();
    HANDLE getLeftButtonEvent();
    HANDLE getRightButtonEvent();
public:
    static WinThread* instance();
    bool start();
    bool stop();
    bool startDragging(const DataContainer& data);
    inline bool showPopupMenu(const DataContainer& data)
    {
        return _window.sendMessage(WM_DND_SHOWPOPUPMENU, 0, reinterpret_cast<LPARAM>(&data))?true:false;
    }
};

#endif // __KARBAZOL_DRAGNDROP_2_0__WINTHRD_H__
// vim: set et ts=4 ai :

