
#include <common/utils.h>
#include <hook/dndcmnct.h>
#include <hook/hook.h>

#include "hldrwnd.h"
#include "holder.h"

WCHAR* HolderWindow::getClassName()
{
    static WCHAR* _class = HOLDER_CLASS_NAME;
    return _class;
}

LRESULT HolderWindow::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_ARE_YOU_HOLDER:
        return HOLDER_YES;
    case WM_FARWINDOWSCREATED:
        if (_holder) _holder->notifyOtherHolder(WM_FARWINDOWSCREATED_FWD, wParam, lParam);
    case WM_FARWINDOWSCREATED_FWD:
        return onFarWindowsCreated((HWND)wParam, (HWND)lParam);
    case WM_DNDWINDOWDESTROY:
        if (_holder) _holder->notifyOtherHolder(WM_DNDWINDOWDESTROY_FWD, wParam, lParam);
    case WM_DNDWINDOWDESTROY_FWD:
        return onDndWindowDestroy((HWND)wParam);
    case WM_HLDR_ISDNDWND:
        return onIsDndWindow((HWND)wParam);
    case WM_HLDR_ISFARWND:
        return onIsFarWindow((HWND)wParam);
    case WM_HLDR_GETDNDWND:
        return onGetDndWindow((HWND)wParam);
    case WM_HLDR_GETFARWND:
        return onGetFarWindow((HWND)wParam);
    case WM_HLDR_SETHOOK:
        return onSetTheHook(wParam?true:false);
    case WM_DESTROY:
        onSetTheHook(false);
        break;
    }
    return MyWindow::handle(msg, wParam, lParam);
}

LRESULT HolderWindow::onSetTheHook(bool value)
{
    if (_holder->setHook(value))
    {
        return HOLDER_YES;
    }
    else
    {
        return HOLDER_NOT;
    }
}

LRESULT HolderWindow::onFarWindowsCreated(HWND hFar, HWND dnd)
{
    ASSERT(_holder);

    _holder->registerDND(hFar, dnd);

    return HOLDER_YES;
}

LRESULT HolderWindow::onDndWindowDestroy(HWND dnd)
{
    ASSERT(_holder);

    _holder->unregisterDND(dnd);

    if (!_holder->farsCount())
    {
        postMessage(WM_QUIT);
    }

    return HOLDER_NOT;
}

LRESULT HolderWindow::onIsDndWindow(HWND dnd)
{
    if (_holder->findDnd(dnd))
        return HOLDER_YES;
    return HOLDER_NOT;
}

LRESULT HolderWindow::onIsFarWindow(HWND hFar)
{
    if (_holder->isFarWindow(hFar))
    {
        return HOLDER_YES;
    }
    return HOLDER_NOT;
}

LRESULT HolderWindow::onGetDndWindow(HWND hFar)
{
    return (LRESULT)_holder->getActiveDnd(hFar);
}

LRESULT HolderWindow::onGetFarWindow(HWND dnd)
{
    return (LRESULT)GetParent(dnd);
}

// vim: set et ts=4 ai :

