/**
 * The author disclaims copyright to this source code.
 * @file mainthrd.cpp
 *
 * $Id: mainthrd.cpp 81 2011-11-07 08:50:02Z Karbazol $
 */

#include <utils.h>
#include <dll.h>

#include "mainthrd.h"
#include "dragging.h"
#include "far.h"
#include "fardlg.h"
#include "dlgfmwk.h"

MainThread::MainThread():_posted(), _sendGuard(), _postGuard()
{
    _threadId = GetCurrentThreadId();
    _eventMessage = CreateEvent(NULL, FALSE, FALSE, NULL);
    _eventProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Initialize the stuff which must be run in mainthread
    RunningDialogs::instance();
}

MainThread::~MainThread()
{
    CloseHandle(_eventProcessed);
    CloseHandle(_eventMessage);
}

MainThread* MainThread::instance()
{
    static MainThread* p = NULL;
    if (!p)
    {
        p = new MainThread();
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

void MainThread::kill(MainThread* p)
{
    if (p)
        delete p;
}

void* MainThread::handleMessage(unsigned int msg, void* param0, void* param1,
        void* /*param2*/, void* /*param3*/)
{
    ASSERT(_threadId == GetCurrentThreadId());
    switch (msg)
    {
    case MTM_SETDRAGGING:
        onSetDragging(!!param0);
        break;
    case MTM_GETDIRFROMPT:
        return (void*)onGetDirFromScreenPoint(*(POINT*)param0, *(MyStringW*)param1);
    case MTM_SENDDLGMSG:
        return (void*)onSendDlgMessage(param0);
    case MTM_CALLTHEOBJECT:
        return onCallIt(reinterpret_cast<void*(*)(void*)>(param0), param1);
    }
    return NULL;
}

void MainThread::postMessage(unsigned int msg, void* param0, void* param1,
        void* param2, void* param3)
{
    LOCKIT(_postGuard);

    Message m = {
        msg, param0, param1, param2, param3
    };

    _posted.append(m);
}

void* MainThread::sendMessage(unsigned int msg, void* param0, void* param1,
        void* param2, void* param3)
{
    if (_threadId == GetCurrentThreadId())
    {
        return _result = handleMessage(msg, param0, param1, param2, param3);
    }
    else
    {

        LOCKIT(_sendGuard);

        _msg._msg = msg;
        _msg._param0 = param0;
        _msg._param1 = param1;
        _msg._param2 = param2;
        _msg._param3 = param3;

        SignalObjectAndWait(_eventMessage, _eventProcessed, INFINITE, FALSE);

        return _result;
    }
}

unsigned int MainThread::processMessage(bool wait, void** result)
{
    if (_threadId != GetCurrentThreadId())
    {
        return 0;
    }

    unsigned int res = 0;

    if (WaitForSingleObject(_eventMessage, wait?INFINITE:0) == WAIT_OBJECT_0)
    {
        _result = handleMessage(_msg._msg, _msg._param0, _msg._param1,
                _msg._param2, _msg._param3);

        if (result)
            *result = _result;

        res = _msg._msg;

        SetEvent(_eventProcessed);
    }

    processPostedMessages();

    return res;
}

bool MainThread::popPostedMessage(MainThread::Message& m)
{
    LOCKIT(_postGuard);
    if (!_posted.size())
        return false;

    m = _posted[0];
    _posted.deleteItem(0);

    return true;
}

void MainThread::processPostedMessages()
{
    Message m = {0};
    while (popPostedMessage(m))
    {
        handleMessage(m._msg, m._param0, m._param1, m._param2, m._param3);
    }

    {
        LOCKIT(_postGuard);

        _posted.clear();
    }

}

void* MainThread::waitForMessage(unsigned int msg)
{
    void* res;
    while (msg != processMessage(true, &res));
    return res;
}

void MainThread::onSetDragging(bool value)
{
    Dragging* dragging = Dragging::instance();
    if (!dragging)
    {
        return;
    }
    if (value)
    {
        dragging->draggingStarted();
    }
    else
    {
        dragging->draggingEnded();
    }
}

bool MainThread::onGetDirFromScreenPoint(POINT&pt, MyStringW& dir)
{
    WindowInfo wi = {sizeof(wi)};
    wi.Pos = -1;

    FarGetWindowInfo(wi);
    if (wi.Type != WTYPE_PANELS)
        return false;

    ScreenToClient(GetFarWindow(), &pt);

    RECT rect;
    GetClientRect(GetFarWindow(), &rect);

    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);

    pt.x = pt.x * ci.srWindow.Right / rect.right;
    pt.y = pt.y * ci.srWindow.Bottom / rect.bottom;

    PanelInfo info;
    if (FarGetActivePanelInfo(info))
    {
        if (pt.x > info.PanelRect.left && pt.x < info.PanelRect.right &&
            pt.y > info.PanelRect.top && pt.y < info.PanelRect.bottom)
        {
            dir = FarGetActivePanelDirectory();
            return true;
        }
    }

    if (FarGetPassivePanelInfo(info))
    {
        if (pt.x > info.PanelRect.left && pt.x < info.PanelRect.right &&
            pt.y > info.PanelRect.top && pt.y < info.PanelRect.bottom)
        {
            dir = FarGetPassivePanelDirectory();
            return true;
        }
    }

    return false;
}

void* MainThread::onCallIt(void* (*function)(void*), void* param)
{
    if (!function)
        return reinterpret_cast<void*>(-1);

    return function(param);
}

LONG_PTR MainThread::onSendDlgMessage(void* msg)
{
    RunningDialogs* runningDialogs = RunningDialogs::instance();
    if (!runningDialogs)
    {
        return 0;
    }
    return runningDialogs->processMessages(reinterpret_cast<RunningDialogs::Message*>(msg));
}

// vim: set et ts=4 ai :

