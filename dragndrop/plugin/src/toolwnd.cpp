/**
 * @file: toolwnd.cpp
 * The file contains implementation of ToolWindow class.
 *
 * $Id: toolwnd.cpp 81 2011-11-07 08:50:02Z Karbazol $
 */

#include <common/utils.h>
#include <dll/dndmsgs.h>
#include <hook/hldrapi.h>

#include "far.h"
#include "toolwnd.h"
#include "winthrd.h"
#include "mainthrd.h"
#include "configure.hpp"
#include "dragbmp.h"
#include "dropprcs.h"
#include "dataobj.h"

void ToolWindow::beforeCreation(DWORD& /*style*/, DWORD& /*styleEx*/)
{
//    style = WS_CHILD;
}

WCHAR* ToolWindow::getClassName()
{
    static WCHAR* _class = L"Karbazol Drag'n'drop tool window";
    return _class;
}

ToolWindow::ToolWindow(): MyWindow(), Unknown(), _data(), _dropData(), _dropHelper(),
    _menu2(), _menu3(),  _mouseCounter(0)
{
    AddRef();

}

ToolWindow::~ToolWindow()
{
}

void ToolWindow::onCreate()
{
    RegisterDragDrop(hwnd(), static_cast<IDropTarget*>(this));

    if (FAILED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                    IID_IDropTargetHelper, (void**)&_dropHelper)))
    {
        TRACE("Failed to create instance of CLSID_DragDropHelper\n");
        _dropHelper = NULL;
    }

    HolderApi* holderApi = HolderApi::instance();

    if (holderApi)
    {
        holderApi->windowsCreated(parent(), hwnd());

        holderApi->setHook(true);
    }
}

LRESULT ToolWindow::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(hwnd());

    switch (msg)
    {
    case WM_PREPAREFORDRAGGING:
        return prepareForDragging(*reinterpret_cast<DataContainer*>(lParam));
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return onMouse(msg, wParam, lParam);
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        return onDblClick(msg, wParam, lParam);
    case WM_MENUCHAR:
        return onMenuChar(msg, wParam, lParam);
    case WM_INITMENUPOPUP:
        // Here we allow the m_pMenu to generate its 'Send To' menu items.
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
        return onMenuMessage(msg, wParam, lParam);

    case WM_ISACTIVEDND:
        return onIsActiveFar();
    case WM_HLDR_ISDNDWND:
        return HOLDER_YES;
    case WM_ISDRAGGIN:
        return onIsDragging();
    case WM_SHOWDNDWINDOW:
        show();
        break;
    case WM_HIDEDNDWINDOW:
        hide();
        break;
    case WM_DND_SHOWPOPUPMENU:
        return showPopupMenu(*reinterpret_cast<const DataContainer*>(lParam));

    case WM_CREATE:
        onCreate();
        break;
    case WM_DESTROY:
        onDestroy();
        break;
    default:
        break;
    }
    return MyWindow::handle(msg, wParam, lParam);
}

LRESULT ToolWindow::onIsDragging()
{
    if (_dropData)
    {
        return HOLDER_YES;
    }
    else
    {
        return HOLDER_NOT;
    }
}

LRESULT ToolWindow::onIsActiveFar()
{
    if (IsActiveFar())
    {
        TRACE("We are active far\n");
        show();
        return HOLDER_YES;
    }
    else
        return HOLDER_NOT;
}

HWND ToolWindow::onGetActiveFar()
{
    if (IsActiveFar())
    {
        show();
        return hwnd();
    }

    hide();

    HolderApi* holderApi = HolderApi::instance();
    if (!holderApi)
    {
        return NULL;
    }

    return holderApi->getActiveDnd(parent());
}

#ifdef _DEBUG
static void dumpConsoleFont()
{
    static bool executed = false;

    if (executed)
        return;

    executed = true;

    HDC hdc = GetDC(GetConsoleWindow());

    char buff[200] = {0};

    GetTextFaceA(hdc, LENGTH(buff), buff);

    ReleaseDC(GetConsoleWindow(), hdc);

    TRACE("Console font: %s\n", buff);
}
#endif

LRESULT ToolWindow::prepareForDragging(const DataContainer& data)
{
#ifdef _DEBUG
    dumpConsoleFont();
#endif

    _data = 0;
    Config* config = Config::instance();
    HolderApi* holderApi = HolderApi::instance();
    if (!config || !holderApi)
    {
        return 0;
    }

    createDataObject(data, &_data, config->useShellObject());

    if (!show())
    {
        return 0;
    }

    // make sure we'll get all mouse events
    setCapture();

    if (holderApi->isLeftButtonDown())
    {
        mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_LEFTDOWN,  0, 0, 0, NULL);
    }

    if (holderApi->isRightButtonDown())
    {
        mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_RIGHTDOWN,  0, 0, 0, NULL);
    }

    return 1;
}

LRESULT ToolWindow::showPopupMenu(const DataContainer& data)
{
    int res = 1;
    HRESULT hr;
    ShPtr<IContextMenu> pMenu;

    hr = getShellUIObject(data, IID_IContextMenu,
            reinterpret_cast<void**>(&pMenu));
    if (FAILED(hr))
        return 0;

    HMENU hMenu = CreatePopupMenu();
    hr = pMenu->QueryContextMenu(hMenu, 1, 1, (UINT)-1, 0);

    if (SUCCEEDED(hr))
    {
        hr = pMenu->QueryInterface(IID_IContextMenu2, reinterpret_cast<void**>(&_menu2));
        if (FAILED(hr))
            TRACE("IContextMenu2 is not supported\n");
        hr = pMenu->QueryInterface(IID_IContextMenu3, reinterpret_cast<void**>(&_menu3));
        if (FAILED(hr))
            TRACE("IContextMenu3 is not supported\n");
        POINT pt;
        GetCursorPos(&pt);
        res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd(), NULL);

        if (res)
        {
            CMINVOKECOMMANDINFO ci = {
                sizeof(ci),
                    0,
                    hwnd(),
                    MAKEINTRESOURCEA(res-1),0,0,SW_NORMAL,0,0
            };

            hr = pMenu->InvokeCommand(&ci);
            if (FAILED(hr))
            {
                res = 0;
            }
        }
    }
    else
    {
        res = 0;
    }

    DestroyMenu(hMenu);
    _menu2 = 0;
    _menu3 = 0;

    return res;
}

HRESULT ToolWindow::QueryInterface(REFIID iid, void** obj)
{
    if (IsBadWritePtr(obj, sizeof(*obj)))
        return E_INVALIDARG;
    if (IsEqualGUID(iid, IID_IDropSource))
    {
        AddRef();
        *obj = static_cast<IDropSource*>(this);
        return S_OK;
    }
    else if (IsEqualGUID(iid, IID_IDropTarget))
    {
        AddRef();
        *obj = static_cast<IDropTarget*>(this);
        return S_OK;
    }

    return Unknown::QueryInterface(iid, obj);
}

HRESULT ToolWindow::QueryContinueDrag(BOOL fEscapePressed, DWORD keyState)
{
    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;
    else if ((keyState & (MK_LBUTTON|MK_RBUTTON)) == 0)
        return DRAGDROP_S_DROP;
    else
        return S_OK;
}
HRESULT ToolWindow::GiveFeedback(DWORD /*dwEffect*/)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

LRESULT ToolWindow::onDblClick(UINT msg, WPARAM wParam, LPARAM lParam)
{
    hide();
    if (hwnd() == GetCapture())
        ReleaseCapture();
    switch (msg)
    {
    case WM_LBUTTONDBLCLK:
        {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
        }
        break;
    default:
        break;
    }
    return MyWindow::handle(msg, wParam, lParam);
}

LRESULT ToolWindow::onMenuChar(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (_menu3)
    {
        LRESULT res;
        _menu3->HandleMenuMsg2(msg, wParam, lParam, &res);
        return res;
    }

    return MyWindow::handle(msg, wParam, lParam);
}

LRESULT ToolWindow::onMenuMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (_menu3)
    {
        LRESULT res;
        _menu3->HandleMenuMsg2(msg, wParam, lParam, &res);
        return res;
    }
    else if (_menu2)
    {
        _menu2->HandleMenuMsg(msg, wParam, lParam);
    }
    return msg == WM_INITMENUPOPUP ? TRUE : FALSE;
}
LRESULT ToolWindow::onMouse(UINT /*msg*/, WPARAM wParam, LPARAM /*lParam*/)
{
    if (_mouseCounter++ < 1)
        return 0;
    _mouseCounter = 0;

    /**
     * We cannot use DragDetect function here because it returns
     * false in case when mouse moves with right button pressed.
     *
    if (!DragDetect(hwnd(), pt))
    {
        MainThread::instance()->setDragging(false);
        hide();
        return 0;
    }
     */
    if (wParam & (MK_LBUTTON|MK_RBUTTON))
    {
        TRACE("onMouse: buttond down\n");
        if ( _data != NULL )
        {
            MainThread::instance()->setDragging(true);
#if 0
            /** @todo Implement support for nice picture during drag */
            ShPtr<IDragSourceHelper> helper;
            DragBitmap theBmp;
            if (_dropHelper && SUCCEEDED(_dropHelper->QueryInterface(IID_IDragSourceHelper, (void**)&helper)))
            {
                SHDRAGIMAGE img;
                theBmp.getSize(img.sizeDragImage);
                img.ptOffset.x = img.ptOffset.y = 0;
                img.hbmpDragImage = theBmp.handle();
                img.crColorKey = RGB(0,0,255);

                helper->InitializeFromBitmap(&img, _data);
            }
#endif
            TRACE("Dragging starting\n");
            DWORD effects=0;
            HRESULT hr = DoDragDrop(_data, this, DROPEFFECT_COPY, &effects);
            DUMPERROR(hr);
            TRACE("Dragging ended\n");
            if (hr == DRAGDROP_S_DROP)
            {
                MainThread::instance()->callIt([](void*)
                        {
                            FarClearSelectionActivePanel(0, FarGetActivePanelItems(true).size());
                            return (void*)nullptr;
                        }, nullptr);
            }
        }

        MainThread::instance()->setDragging(false);

        _data = nullptr;
    }
    else
    {
        MainThread::instance()->setDragging(false);
        if (hwnd() == GetCapture())
            ReleaseCapture();
    }
    hide();

    return 0;
}

void ToolWindow::onDestroy()
{
    _dropHelper = nullptr;

    RevokeDragDrop(hwnd());
    HolderApi::instance()->windowsDestroy(hwnd());
}

bool ToolWindow::show()
{
    RECT rect;

    if (!GetWindowRect(parent(), &rect))
    {
        LASTERROR();
        return false;
    }

    if (!SetWindowPos(hwnd(), HWND_TOP, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            SWP_SHOWWINDOW|SWP_NOACTIVATE)||
        !SetWindowPos(hwnd(), HWND_TOP, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOACTIVATE))
    {
        LASTERROR();
        return false;
    }

    return true;
}

bool ToolWindow::hide()
{
    return !!ShowWindow(hwnd(), SW_HIDE);
}

HRESULT ToolWindow::DragEnter(IDataObject* obj, DWORD keyState, POINTL pt, DWORD* effect)
{
    _dropData = obj;

    if (_dropHelper)
    {
        _dropHelper->DragEnter(hwnd(), obj, (POINT*)&pt, *effect);
    }

    if (DropProcessor::instance()->canProcess(obj) != S_OK)
        *effect = DROPEFFECT_NONE;

    return DragOver(keyState, pt, effect);
}

HRESULT ToolWindow::DragOver(DWORD keyState, POINTL pt, DWORD* effect)
{
    if (IsBadWritePtr(effect, sizeof(*effect)))
        return E_POINTER;

    if (*effect != DROPEFFECT_NONE)
    {
        // We can handle it
        keyStateToEffect(keyState, *effect);
    }

    /** @todo Check wheather the files are dragged over the panel
     with the same path. Files should not be dropped on them selves. */

    if (_dropHelper)
    {
        _dropHelper->DragOver((POINT*)&pt, *effect);
    }

    return S_OK;
}

HRESULT ToolWindow::DragLeave()
{
    if (_dropHelper)
    {
        _dropHelper->DragLeave();
    }
    _dropData = NULL;
    hide();
    return S_OK;
}

HRESULT ToolWindow::Drop(IDataObject* obj, DWORD keyState, POINTL ptl, DWORD* effect)
{
    TRACE("Drop occured\n");
    _dropData = NULL;
    if (_dropHelper)
    {
        _dropHelper->Drop(obj, (POINT*)&ptl, *effect);
    }

    if (*effect == DROPEFFECT_NONE)
        return S_OK;

    POINT pt = {ptl.x, ptl.y};

    MyStringW dir;
    if (!MainThread::instance()->getDirFromScreenPoint(pt, dir))
    {
        *effect = DROPEFFECT_NONE;
        return S_OK;
    }

    keyStateToEffect(keyState, *effect);

    hide();

    DropProcessor* dropProcessor = DropProcessor::instance();

    if (!dropProcessor)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = dropProcessor->processDrop(obj, effect, dir);

    return hr;
}

void ToolWindow::keyStateToEffect(DWORD /*keyState*/, DWORD& /*effect*/)
{
    /** @todo Handle differrent scenarios: move/copy/link. bug #6 */
}

// vim: set et ts=4 ai :

