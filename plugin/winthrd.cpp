#include "far.h"
#include "dll.h"
#include "winthrd.h"
#include "dragging.h"
#include "mainthrd.h"
#include "dndmsgs.h"
#include "hldrapi.h"

WinThread* WinThread::instance()
{
    static WinThread* p = 0;
    if (!p)
    {
        p = new WinThread;
        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
    }
    return p;
}

void WinThread::kill(WinThread* p)
{
    if (p)
        delete p;
}

bool WinThread::start()
{
    if (_handle)
        return false;
    
    _handle = CreateThread(NULL, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(&runInstnace),
            this, 0, &_id);

    if (_handle)
    {
        MainThread::instance()->waitForMessage(MTM_WINTHRSTARTED);
        return true;
    }
    else
    {
        return false;
    }
}

bool WinThread::stop()
{
    if (!_handle)
        return false;

    if (!PostThreadMessage(_id, WM_QUIT, 0, 0))
        return false;

    if (WaitForSingleObject(_handle, INFINITE) != WAIT_OBJECT_0)
        return false;

    CloseHandle(_handle);
    _handle = NULL;

    return true;
}

DWORD WinThread::runInstnace(WinThread* p)
{
    p->run();
    return 0;
}

void WinThread::run()
{
    OleInitialize(NULL);

    _window.create(GetFarWindow());

    MainThread::instance()->winThreadStarted();

    for(;;)
    {
        MSG msg;
        DWORD res;

        res = GetMessageW(&msg, 0, 0, 0);
        if (!res || res == -1)
        {
            _window.destroy();
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    OleUninitialize();
}

bool WinThread::startDragging(IDataObject* data)
{
    if (HolderApi::instance()->isLeftButtonDown())
        mouse_event(MOUSEEVENTF_LEFTUP,  0, 0, 0, NULL);
    if (HolderApi::instance()->isRightButtonDown())
        mouse_event(MOUSEEVENTF_RIGHTUP,  0, 0, 0, NULL);
    if (_window.sendMessage(WM_PREPAREFORDRAGGING, 0, (LPARAM)data))
    {
        //mouse_event(MOUSEEVENTF_LEFTDOWN,  0, 0, 0, NULL);

        MainThread::instance()->waitForMessage(MTM_SETDRAGGING);
        return Dragging::instance()->dragging();
    }

    return false;
}

// vim: set et ts=4 ai :

