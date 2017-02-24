#include <common/utils.h>
#include <hook/hldrapi.h>
#include <hook/dndcmnct.h>

#include "thrdfltr.h"

void ThreadFilter::handle(MSG& msg)
{
    if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
    {
        if (msg.hwnd == GetCapture())
        {
            if (_noDragging)
                return;

            checkDndUnderMouse(msg.hwnd);
        }
        else
        {
            reset();
        }
    }
}

void ThreadFilter::reset()
{
    _noDragging = false;
    _draggingChecked = false;
    _dragCheckCounter = 5;
}

void ThreadFilter::checkDragging(HWND hwnd)
{
    if (!_draggingChecked)
    {
        if (_dragCheckCounter)
        {
            _dragCheckCounter--;
            return;
        }

        _noDragging = !isDndDropTarget(hwnd);
        if (_noDragging)
            hideDndWindow(hwnd);

        _draggingChecked = true;
    }

    return;
}

void ThreadFilter::checkDndUnderMouse(HWND msgHwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HWND res = WindowFromPoint(pt);

    if (res == msgHwnd)
    {
        // Cursor is over the window that recieves input.
        // No need to check further
        return;
    }

    if (isDndWindow(res))
    {
        // This mean there is already DND window under the cursor
        checkDragging(res);
        return;
    }

    HolderApi* holderApi = HolderApi::instance();
    if (!holderApi)
    {
        return;
    }
    res = holderApi->isFarWindow(res);
    if (!res)
    {
        return;
    }

    res = holderApi->getActiveDnd(res);
    if (res && IsWindow(res))
    {
        showDndWindow(res);
    }
}

// vim: set et ts=4 ai :

