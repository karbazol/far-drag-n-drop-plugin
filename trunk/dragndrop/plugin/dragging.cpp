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

/**
 * @brief Start dragging.
 *
 * The function checks the follwing conditions and decides
 * whether to start dragging.
 */
bool Dragging::start()
{
    TRACE("Should we start dragging?\n");

    if (_dragging)
    {
        /** Dragging can be only performed only if it is not already going */
        return false;
    }

    if (FAR_CONSOLE_FULLSCREEN == ConsoleMode(FAR_CONSOLE_GET_MODE))
    {
        /**
         * We cannot start dragging when Far is running in hardware console mode.
         * (Beginning from Windows Vista such mode is not supported).
         */
        return false;
    }

    WindowInfoW wi;
    wi.Pos = -1;

    if (!FarGetWindowInfo(wi) || wi.Type != WTYPE_PANELS)
        return false;

    PanelInfoW info;
    if (!FarGetActivePanelInfo(info) || info.Plugin && !(info.Flags & PFLAGS_REALNAMES))
        return false;

    if (!info.Visible)
        return false;

    if (!InputProcessor::instance()->isMouseWithinRect(info.PanelRect.left, info.PanelRect.top,
         info.PanelRect.right, info.PanelRect.bottom))
        return false;

    MyStringW dir = FarGetActivePanelDirectory();
    PluginPanelItemsW items = FarGetActivePanelItems(true);

    ShPtr<IDataObject> dataObj;
    if (FAILED(createDataObject(dir, items, &dataObj)))
        return false;

    /** @todo Find a place to show pop-up menu */
    if (WinThread::instance()->startDragging(dataObj))
        _dragging = true;

    return _dragging;
}

// vim: set et ts=4 ai :

