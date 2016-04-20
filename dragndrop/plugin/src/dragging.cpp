#include <shlobj.h>
#include <common/myshptr.h>
#include <common/utils.h>
#include <dll/dll.h>

#include "dragging.h"
#include "far.h"
#include "inpprcsr.h"
#include "winthrd.h"
#include "dataobj.h"
#include "datacont.h"
#include "configure.hpp"

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
        if (p)
        {
            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(kill), p);
            }
        }
    }

    return p;
}

void Dragging::initialize()
{
    if (!_initialized)
    {
        _initialized = true;
        WinThread* winThread = WinThread::instance();
        if (winThread)
        {
            winThread->start();
        }
    }
}

void Dragging::shutDown()
{
    WinThread* winThread = WinThread::instance();
    if (winThread)
    {
        winThread->stop();
    }
}

bool Dragging::isReadyForDragging()
{
    if (!_initialized)
        return false;

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

    WindowInfo wi = {sizeof(wi)};
    wi.Pos = -1;

    if (!FarGetWindowInfo(wi) || wi.Type != WTYPE_PANELS)
        return false;

    PanelInfo info;
    if (!FarGetActivePanelInfo(info) || info.PluginHandle && !(info.Flags & PFLAGS_REALNAMES))
        return false;

    if (!(info.Flags & PFLAGS_VISIBLE))
        return false;

    return true;
}

/**
 * @brief Start dragging.
 *
 * The function checks the follwing conditions and decides
 * whether to start dragging:
 * 1. Active Far window should be "Panels", not "Editor" and not "Viewer"
 * 2. Mouse pointer should be over panels and not over status bar and console.
 */
bool Dragging::start()
{
    TRACE("Should we start dragging?\n");

    if (_dragging)
    {
        return false;
    }

    WinThread* winThread = WinThread::instance();
    InputProcessor* inputProcessor = InputProcessor::instance();

    if (!winThread || !inputProcessor)
    {
        return false;
    }

    PanelInfo info;
    if (!FarGetActivePanelInfo(info) || info.PluginHandle && !(info.Flags & PFLAGS_REALNAMES))
    {
        return false;
    }

    if (!inputProcessor->isMouseWithinRect(info.PanelRect.left, info.PanelRect.top,
         info.PanelRect.right, info.PanelRect.bottom))
    {
        return false;
    }

    MyStringW dir = FarGetActivePanelDirectory();
    PluginPanelItemsW items = FarGetActivePanelItems(true);

    ShPtr<IDataObject> dataObj;
    DataContainer* data = new DataContainer(dir, items);

    if (!data)
    {
        return false;
    }

    /* WinThread::startDragging will post a message for tool window.
       When the messages will be picked up in the windows thread the
       ToolWindow::prepareForDragging will be called. */
    if (winThread->startDragging(*data))
    {
        _dragging = true;
    }

    delete data;

    return _dragging;
}

bool Dragging::showPopupMenu()
{
    Config* config = Config::instance();
    if (config && config->showMenu())
    {
        DataContainer data(FarGetActivePanelDirectory(), FarGetActivePanelItems(true));

        WinThread* winThread = WinThread::instance();
        if (!winThread)
        {
            return false;
        }
        return winThread->showPopupMenu(data);
    }
    return false;
}

// vim: set et ts=4 ai :

