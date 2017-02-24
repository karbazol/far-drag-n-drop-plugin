#include <oleholder.hpp>
#include <dll.h>
#include <dndmsgs.h>

#include "far.h"
#include "winthrd.h"
#include "dragging.h"
#include "mainthrd.h"

WinThread::WinThread(): _handle(0), _leftButtonEvent(0), _rightButtonEvent(0),
    _id(0), _window()
{
    HolderApi* holderApi = HolderApi::instance();
    if (holderApi)
    {
        _leftButtonEvent = OpenEvent(SYNCHRONIZE, FALSE, HOLDER_LEFT_EVENT);
        _rightButtonEvent = OpenEvent(SYNCHRONIZE, FALSE, HOLDER_RIGHT_EVENT);
        holderApi->setHolder(static_cast<IHolder*>(this));
    }
}

WinThread::~WinThread()
{
    stop();

    HolderApi* holderApi = HolderApi::instance();

    if (holderApi)
    {
        holderApi->setHolder(NULL);
    }

    CloseHandle(_leftButtonEvent);
    CloseHandle(_rightButtonEvent);
}

const wchar_t* WinThread::getHolderWindowClassName()
{
    return HOLDER_CLASS_NAME;
}

const wchar_t* WinThread::getHolderFileName()
{
    return HOLDER_EXECUTABLE;
}

const wchar_t* WinThread::getHolderMutexName()
{
    return HOLDER_MUTEX;
}

HANDLE WinThread::getLeftButtonEvent()
{
    return _leftButtonEvent;
}

HANDLE WinThread::getRightButtonEvent()
{
    return _rightButtonEvent;
}

WinThread* WinThread::instance()
{
    static WinThread* p = 0;
    if (!p)
    {
        p = new WinThread;
        if (p)
        {
            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
            }
        }
    }
    return p;
}

void WinThread::kill(WinThread* p)
{
    if (p)
    {
        delete p;
    }
}

bool WinThread::start()
{
    MainThread* mainThread = MainThread::instance();
    if (!mainThread || _handle)
        return false;

    _handle = CreateThread(NULL, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(&runInstnace),
            this, 0, &_id);

    if (_handle)
    {
        mainThread->waitForMessage(MTM_WINTHRSTARTED);
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
    OleHolder oleHolder;

    _window.create(GetFarWindow());

    MainThread::instance()->winThreadStarted();

    for (;;)
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
}

/**
 * The functions sends a message to the tool window.
 */
bool WinThread::startDragging(const DataContainer& data)
{
    HolderApi* holderApi = HolderApi::instance();
    MainThread* mainThread = MainThread::instance();
    Dragging* dragging = Dragging::instance();
    if (!holderApi || !mainThread || !dragging)
    {
        return false;
    }

    if (holderApi->isLeftButtonDown())
    {
        mouse_event(MOUSEEVENTF_LEFTUP,  0, 0, 0, NULL);
    }

    if (holderApi->isRightButtonDown())
    {
        mouse_event(MOUSEEVENTF_RIGHTUP,  0, 0, 0, NULL);
    }

    /** See ToolWindow::prepareForDragging for refernce. */
    if (_window.sendMessage(WM_PREPAREFORDRAGGING, 0, (LPARAM)&data))
    {
        //mouse_event(MOUSEEVENTF_LEFTDOWN,  0, 0, 0, NULL);

        mainThread->waitForMessage(MTM_SETDRAGGING);
        return dragging->dragging();
    }

    return false;
}

// vim: set et ts=4 ai :

