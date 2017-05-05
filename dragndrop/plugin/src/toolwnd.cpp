/**
 * @file: toolwnd.cpp
 * The file contains implementation of ToolWindow class.
 *
 */

#include <utils.h>
#include <dndmsgs.h>
#include <hldrapi.h>
#include <VersionHelpers.h>

#include "far.h"
#include "ddlng.h"
#include "toolwnd.h"
#include "winthrd.h"
#include "mainthrd.h"
#include "configure.hpp"
#include "dragbmp.h"
#include "dropprcs.h"
#include "dataobj.h"
#include "shutils.h"

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
    _theDragFile.length(0);
    if (data.fileCount() == 1)
        _theDragFile = data.dir() / data.files()[0];

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

static void setPreferredDropEffect(IDataObject* dropSource, DWORD effect)
{
    FORMATETC format = {CF_PREFERREDDROPEFFECT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {TYMED_HGLOBAL, nullptr, nullptr};
    medium.hGlobal = GlobalAlloc(GMEM_FIXED, sizeof(DWORD));
    if (!medium.hGlobal)
        return;
    *reinterpret_cast<DWORD*>(medium.hGlobal) = effect;
    dropSource->SetData(&format, &medium, TRUE);
}

static BOOL isAsyncOperationGoing(IDataObject* dropSource)
{
    IAsyncOperation* async = nullptr;
    HRESULT result = dropSource->QueryInterface(&async);
    if (FAILED(result))
        return FALSE;
    BOOL inOperation = FALSE;
    result = async->InOperation(&inOperation);
    async->Release();
    if (FAILED(result))
        return FALSE;
    return inOperation;
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
            ShPtr<IDragSourceHelper> helper;
            if (_dropHelper && SUCCEEDED(_dropHelper->QueryInterface(IID_IDragSourceHelper, (void**)&helper)))
            {
                if (IsWindowsVistaOrGreater())
                {
                    // on Vista+, just let the shell generate drag image;
                    // manual XP-style processing works here too,
                    // but shell handles multiple-files situation better,
                    // and for single-file the result is closer to standard
                    helper->InitializeFromWindow(NULL, NULL, _data);
                }
                else
                {
                    // on XP, InitializeFromWindow() does not generate anything by itself,
                    // so create the image manually
                    SHDRAGIMAGE img;
                    if (generateDragImage(_theDragFile, img))
                        helper->InitializeFromBitmap(&img, _data);
                }
            }
            setPreferredDropEffect(_data, DROPEFFECT_COPY);
            TRACE("Dragging starting\n");
            DWORD effects=0;
            HRESULT hr = DoDragDrop(_data, this, DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK, &effects);
            DUMPERROR(hr);
            TRACE("Dragging ended\n");
            if (hr == DRAGDROP_S_DROP)
            {
                if (!isAsyncOperationGoing(_data) && effects == DROPEFFECT_MOVE)
                    deleteFilesFromDataObj(_data);
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

    _possibleDropActions = DropProcessor::instance()->canProcess(obj);

    return DragOver(keyState, pt, effect);
}

HRESULT ToolWindow::DragOver(DWORD keyState, POINTL pt, DWORD* effect)
{
    _dropKeyState = keyState;
    if (IsBadWritePtr(effect, sizeof(*effect)))
        return E_POINTER;

    *effect &= _possibleDropActions;
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

HRESULT ToolWindow::Drop(IDataObject* obj, DWORD /*keyState*/, POINTL ptl, DWORD* effect)
{
    TRACE("Drop occured\n");
    _dropData = NULL;
    if (_dropHelper)
    {
        // do it here to avoid interference between dragging icon and TrackPopupMenu
        _dropHelper->Drop(obj, (POINT*)&ptl, DROPEFFECT_NONE);
    }
    *effect &= _possibleDropActions;
    if (*effect != DROPEFFECT_NONE)
    {
        // we can't rely on keyState passed as an arg, because MK_LBUTTON+MK_RBUTTON are already cleared here
        // use state saved in DragEnter/DragOver
        if (_dropKeyState & MK_RBUTTON)
        {
            HMENU hMenu = CreatePopupMenu();
            if (!hMenu)
                return E_FAIL;
            bool appendOk = true;
            if (*effect & DROPEFFECT_COPY)
                appendOk = appendOk && AppendMenu(hMenu, MF_STRING, DROPEFFECT_COPY, GetMsg(MCopyVerb));
            if (*effect & DROPEFFECT_MOVE)
                appendOk = appendOk && AppendMenu(hMenu, MF_STRING, DROPEFFECT_MOVE, GetMsg(MMoveVerb));
            if (*effect & DROPEFFECT_LINK)
                appendOk = appendOk && AppendMenu(hMenu, MF_STRING, DROPEFFECT_LINK, GetMsg(MLinkVerb));
            appendOk = appendOk && AppendMenu(hMenu, MF_SEPARATOR, -1, nullptr);
            appendOk = appendOk && AppendMenu(hMenu, MF_STRING, DROPEFFECT_NONE, GetMsg(MCancel));
            if (!appendOk)
            {
                DestroyMenu(hMenu);
                return E_FAIL;
            }
            DWORD defaultEffect = *effect;
            keyStateToEffect(_dropKeyState, defaultEffect);
            SetMenuDefaultItem(hMenu, defaultEffect, FALSE);
            DWORD selectedEffect = TrackPopupMenu(hMenu, TPM_RETURNCMD, ptl.x, ptl.y, 0, hwnd(), nullptr);
            DestroyMenu(hMenu);
            if (selectedEffect != DROPEFFECT_COPY && selectedEffect != DROPEFFECT_MOVE && selectedEffect != DROPEFFECT_LINK)
                selectedEffect = DROPEFFECT_NONE;
            *effect = selectedEffect;
        }
        else
        {
            keyStateToEffect(_dropKeyState, *effect);
        }
    }

    if (*effect == DROPEFFECT_NONE)
    {
        TRACE("Drop is not processed because of DROPEFFECT_NONE\n");
        return S_OK;
    }

    POINT pt = {ptl.x, ptl.y};

    MyStringW dir;
    if (!MainThread::instance()->getDirFromScreenPoint(pt, dir))
    {
        TRACE("Could not get dir from screen point\n");
        *effect = DROPEFFECT_NONE;
        return S_OK;
    }

    hide();

    DropProcessor* dropProcessor = DropProcessor::instance();

    if (!dropProcessor)
    {
        TRACE("Could not get drop processor\n");
        return E_OUTOFMEMORY;
    }

    TRACE("Processing drop\n");
    HRESULT hr = dropProcessor->processDrop(obj, effect, dir);

    return hr;
}

void ToolWindow::keyStateToEffect(DWORD keyState, DWORD& effect)
{
    switch (keyState & (MK_SHIFT | MK_CONTROL | MK_ALT))
    {
    case MK_SHIFT:
        if (effect & DROPEFFECT_MOVE)
            effect = DROPEFFECT_MOVE;
        break;
    case MK_CONTROL:
        if (effect & DROPEFFECT_COPY)
            effect = DROPEFFECT_COPY;
        break;
    case MK_SHIFT | MK_CONTROL:
    case MK_ALT:
        if (effect & DROPEFFECT_LINK)
            effect = DROPEFFECT_LINK;
        break;
    }
    // if no control keys are pressed, prefer copy > move > link
    if (effect & DROPEFFECT_COPY)
        effect = DROPEFFECT_COPY;
    else if (effect & DROPEFFECT_MOVE)
        effect = DROPEFFECT_MOVE;
    else if (effect & DROPEFFECT_LINK)
        effect = DROPEFFECT_LINK;
    else
        effect = DROPEFFECT_NONE;
}

// vim: set et ts=4 ai :

