#ifndef __KARBAZOL_DRAGNDROP_2_0__WINTHRD_H__
#define __KARBAZOL_DRAGNDROP_2_0__WINTHRD_H__

#include <windows.h>
#include "toolwnd.h"

class WinThread
{
private:
    HANDLE _handle;
    DWORD _id;
    ToolWindow _window;
    WinThread(): _handle(0), _id(0), _window(){}
    WinThread(const WinThread&);
    ~WinThread()
    {
        stop();
    }
    static DWORD WINAPI runInstnace(WinThread* p);
    static void kill(WinThread* p);
    void run();
public:
    static WinThread* instance();
    bool start();
    bool stop();
    bool startDragging(IDataObject* data);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__WINTHRD_H__
// vim: set et ts=4 ai :

