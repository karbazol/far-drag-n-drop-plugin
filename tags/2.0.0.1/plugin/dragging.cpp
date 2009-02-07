#include <shlobj.h>
#include "dragging.h"
#include "dll.h"
#include "far.h"
#include "utils.h"
#include "inpprcsr.h"
#include "winthrd.h"
#include "dataobj.h"
#include "myshptr.h"

void Dragging::kill(Dragging* p)
{
    if (p)
        delete p;
}

Dragging* Dragging::instance()
{
    static Dragging* p = 0;

    if (!p)
    {
        p = new Dragging();
        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(kill), p);
    }

    return p;
}

void Dragging::initialize()
{
    if (!_initialized)
    {
        _initialized = true;
        WinThread::instance()->start();
    }
}

void Dragging::shutDown()
{
    WinThread::instance()->stop();
}

bool Dragging::start()
{
    TRACE("Should we start dragging?\n");

    if (_dragging)
        return false;

    if (FAR_CONSOLE_FULLSCREEN == ConsoleMode(FAR_CONSOLE_GET_MODE))
        return false;

    struct WindowInfo wi;
    wi.Pos = -1;

    if (!FarGetWindowInfo(&wi) || wi.Type != WTYPE_PANELS)
        return false;

    struct PanelInfo info;
    if (!FarGetPanelInfo(&info) || info.Plugin && !(info.Flags & PFLAGS_REALNAMES))
        return false;

    if (!info.Visible)
        return false;

    if (!InputProcessor::instance()->isMouseWithinRect(info.PanelRect.left, info.PanelRect.top,
         info.PanelRect.right, info.PanelRect.bottom))
        return false;

    ShPtr<IDataObject> dataObj;
    if (FAILED(createDataObject(info, &dataObj)))
        return false;

    if (WinThread::instance()->startDragging(dataObj))
        _dragging = true;

    return _dragging;
#if 0
    /**
    LPWSTR p = PanelItemToWidePath(info.CurDir, *info.SelectedItems);
    if (p)
    {
        SysFreeString(p);
    }
    /**/

    return false;
#endif
}

// vim: set et ts=4 ai :

