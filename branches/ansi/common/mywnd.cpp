/**
 * @file mywnd.cpp
 * The file contains implementation of MyWindow class.
 *
 * $Id$
 */
#include "mywnd.h"
#include "utils.h"

void MyWindow::beforeCreation(DWORD& /*style*/,DWORD& /*styleEx*/)
{
}

bool MyWindow::create(HWND parent)
{
    ASSERT(_hwnd == NULL && _parent == NULL);

    if (!createClass())                              
        return false;

    _parent = parent;
    DWORD styleEx = WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE;
    DWORD style = WS_POPUP;
    beforeCreation(style, styleEx);
    _hwnd = CreateWindowExW(styleEx, getClassName(), L"", style, 
            0, 0, 50, 50, parent, 0, GetModuleHandle(NULL), this);

    return _hwnd?true:false;
}

bool MyWindow::initClass(WNDCLASSEX& cls)
{
    cls.cbSize = sizeof(cls);
    cls.style = 0;
    cls.lpfnWndProc = &wndProc;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = sizeof(LONG_PTR);
    cls.hInstance = GetModuleHandle(NULL);
    cls.hIcon = NULL;
    cls.hCursor = LoadCursor(NULL, IDC_ARROW);
#ifndef _DEBUG
    cls.hbrBackground = NULL;
#else
    cls.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
#endif
    cls.lpszMenuName = NULL;
    cls.lpszClassName = getClassName();
    cls.hIconSm = NULL;

    return true;
}

bool MyWindow::destroy()
{
    if (!_hwnd)
        return false;

    bool res = DestroyWindow(_hwnd)?true:false;

    _hwnd = NULL;

    return res;
}

bool MyWindow::createClass()
{
    WNDCLASSEXW cls;
    if (GetClassInfoExW(GetModuleHandle(NULL), getClassName(), &cls))
        return true;

    if (!initClass(cls))
        return false;
    
    return RegisterClassExW(&cls)?true:false;
}

MyWindow* MyWindow::getThis(HWND hwnd)
{
    if (!hwnd)
        return 0;

    return reinterpret_cast<MyWindow*>(GetWindowLongPtr(hwnd, 0));
}

LRESULT MyWindow::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE)
    {
        LPCREATESTRUCTW info = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        MyWindow* This = reinterpret_cast<MyWindow*>(info->lpCreateParams);
        This->_hwnd = hwnd;
        SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(This));
        return This->handle(msg, wParam, lParam);
    }
    else
    {
        MyWindow* This = getThis(hwnd);
        if (This && This->_hwnd == hwnd)
        {
            return This->handle(msg, wParam, lParam);
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT MyWindow::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(_hwnd, msg, wParam, lParam);
}

// vim: set et ts=4 ai :

