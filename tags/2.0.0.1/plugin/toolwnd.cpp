/**
 * @file: The file contains implementation of ToolWindow class.
 * 
 * $Id$
 */

#include "far.h"
#include "utils.h"
#include "toolwnd.h"
#include "winthrd.h"
#include "mainthrd.h"
#include "hldrapi.h"
#include "configure.hpp"
#include "dragbmp.h"
#include "dndmsgs.h"
#include "hldrapi.h"
#include "dropprcs.h"

void ToolWindow::beforeCreation(DWORD& /*style*/, DWORD& /*styleEx*/)
{
//    style = WS_CHILD;
}

WCHAR* ToolWindow::getClassName()
{
    static WCHAR* _class = L"Karbazol Drag'n'drop tool window";
    return _class;
}

ToolWindow::ToolWindow(): MyWindow(), Unknown(), _data(), _dropData(), _dropHelper(), _mouseCounter(0)
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

    HolderApi::instance()->windowsCreated(parent(), hwnd());

    if (Config::instance()->allowDrop())
    {
        HolderApi::instance()->setHook(true);
    }
}

LRESULT ToolWindow::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(hwnd());

    switch (msg)
    {
    case WM_PREPAREFORDRAGGING:
        return prepareForDragging((IDataObject*)lParam);
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return onMouse(msg, wParam, lParam);
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        return onDblClick(msg, wParam, lParam);
    //case WM_GETACTIVEDND:
    //    return (LRESULT)onGetActiveFar();
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

    return HolderApi::instance()->getActiveDnd(parent());
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

LRESULT ToolWindow::prepareForDragging(IDataObject* data)
{
#ifdef _DEBUG
    dumpConsoleFont();
#endif 
    if (!data)
        return 0;

    _data = data;

    if (!show())
        return 0;

    // make sure we'll get all mouse events
    setCapture();

    if (HolderApi::instance()->isLeftButtonDown())
        mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_LEFTDOWN,  0, 0, 0, NULL);
    if (HolderApi::instance()->isRightButtonDown())
        mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_RIGHTDOWN,  0, 0, 0, NULL);

    return 1;
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
    else if ((keyState & MK_LBUTTON) == 0)
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

LRESULT ToolWindow::onMouse(UINT /*msg*/, WPARAM wParam, LPARAM /*lParam*/)
{
    if (_mouseCounter++ < 1)
        return 0;
    _mouseCounter = 0;
    if (wParam & (MK_LBUTTON|MK_RBUTTON))
    {
        TRACE("onMouse: buttond down\n");
        if ( _data != NULL )
        {
            MainThread::instance()->setDragging(true);
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
            TRACE("Dragging starting\n");
            DWORD effects=0;
#ifdef _DEBUG
            HRESULT hr =
#endif
            DoDragDrop(_data, this, DROPEFFECT_COPY, &effects);
#ifdef _DEBUG
            DUMPERROR(hr);
#endif
            TRACE("Dragging ended\n");
            MainThread::instance()->setDragging(false);
        }
        else
            MainThread::instance()->setDragging(false);

        _data = NULL;
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
    _dropHelper = NULL;

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
    return ShowWindow(hwnd(), SW_HIDE)?true:false;
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

    HRESULT hr = DropProcessor::instance()->processDrop(obj, effect, dir);

    hide();

    return hr;
}

void ToolWindow::keyStateToEffect(DWORD /*keyState*/, DWORD& /*effect*/)
{
}

// vim: set et ts=4 ai :

