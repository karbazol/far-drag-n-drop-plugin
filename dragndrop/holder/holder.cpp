#include "holder.h"
#include "utils.h"
#include "dndcmnct.h"
#include "hook.h"

Holder::Holder(): _window(), _mutex(0), _fars()
{
    ASSERT(_instance == 0);
    _instance = this;

    _window.holder(this);

    SECURITY_ATTRIBUTES sec =
    {
        sizeof(sec),
        getEveryOneDescriptor(),
        FALSE
    };

    _leftEvent = CreateEvent(&sec, TRUE, FALSE, HOLDER_LEFT_EVENT);
    _rightEvent = CreateEvent(&sec, TRUE, FALSE, HOLDER_RIGHT_EVENT);

    _llMouse = SetWindowsHookEx(WH_MOUSE_LL, &mouseHook, (HMODULE)getMyModuleBaseAddress(), 0);

    freeEveryOneDescriptor(sec.lpSecurityDescriptor);
}

Holder::~Holder()
{
    UnhookWindowsHookEx(_llMouse);
    CloseHandle(_leftEvent);
    CloseHandle(_rightEvent);
    if (_mutex)
    {
        CloseHandle(_mutex);
        _mutex = NULL;
    }

    _instance = 0;
}

Holder* Holder::_instance = 0;

LRESULT Holder::mouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (!reinterpret_cast<PMSLLHOOKSTRUCT>(lParam)->flags & LLMHF_INJECTED)
        {
            switch (wParam)
            {
            case WM_LBUTTONDOWN:
                TRACE("WM_LBUTTONDOWN\n");
                SetEvent(_instance->_leftEvent);
                break;
            case WM_RBUTTONDOWN:
                TRACE("WM_RBUTTONDOWN\n");
                SetEvent(_instance->_rightEvent);
                break;
            case WM_LBUTTONUP:
                TRACE("WM_LBUTTONUP\n");
                ResetEvent(_instance->_leftEvent);
                break;
            case WM_RBUTTONUP:
                TRACE("WM_RBUTTONUP\n");
                ResetEvent(_instance->_rightEvent);
                break;
            default:
                break;
            }

        }
    }
    return CallNextHookEx(_instance->_llMouse, nCode, wParam, lParam);
}

bool Holder::initialize()
{
    if (!setupCurDir())
        return false;

    if (!createMutex())
        return false;

    if (!_window.create(NULL))
        return false;

    return true;
}

int Holder::run()
{
    for (;;)
    {
        MSG msg;
        DWORD res = GetMessage(&msg, 0, 0, 0);
        if (!res || res == -1)
        {
            _window.destroy();
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

bool Holder::setupCurDir()
{
    size_t buff = ExpandEnvironmentStrings(L"%SystemRoot%", NULL, 0);

    if (!buff)
        return false;

    WCHAR *currDir = new WCHAR[buff];

    bool res = false;
    if (ExpandEnvironmentStrings(L"%SystemRoot%", currDir, 
                static_cast<DWORD>(buff)))
    {
        res = SetCurrentDirectory(currDir)?true:false;
    }

    delete [] currDir;

    return res;
}

bool Holder::createMutex()
{
    ASSERT(_mutex == NULL);

    SECURITY_ATTRIBUTES sec={
        sizeof(sec),
        getEveryOneDescriptor(),
        FALSE
    };

    _mutex = CreateMutex(&sec, TRUE, HOLDER_MUTEX);
    
    freeEveryOneDescriptor(sec.lpSecurityDescriptor);

    if (!_mutex)
        return false;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return false;

    return true;
}

void Holder::registerDND(HWND hFar, HWND dnd)
{
    FarItem* item = findFar(hFar, true);

    item->hwnd(hFar);
    item->dnds().append(dnd);
}

void Holder::unregisterDND(HWND dnd)
{
    size_t i;
    for (i = 0 ; i < _fars.size(); i++)
    {
        while (!IsWindow(_fars[i].hwnd()))
        {
            if (i >= _fars.deleteItem(i))
                return;
        }

        size_t j;

        for (j = 0; j < _fars[i].dnds().size(); j++)
        {
            while (!IsWindow(_fars[i].dnds()[j])||dnd == _fars[i].dnds()[j])
            {
                if (j >= _fars[i].dnds().deleteItem(j))
                    break;
            }

            if (!_fars[i].dnds().size())
            {
                if (i >= _fars.deleteItem(i))
                    return;
            }
        }
    }
}

FarItem* Holder::findFar(HWND hFar, bool append)
{
    size_t i;

    for (i = 0; i < _fars.size(); i++)
    {
        if (_fars[i].hwnd() == hFar)
            return &_fars[i];
    }

    if (append)
    {
        return &_fars.append(hFar);
    }

    return NULL;
}

FarItem* Holder::findDnd(HWND dnd, size_t* dndIndex)
{
    size_t i;

    for (i = 0; i < _fars.size(); i++)
    {
        size_t j;
        for (j = 0; j < _fars[i].dnds().size(); j++)
        {
            if (_fars[i].dnds()[j] == dnd)
            {
                if (dndIndex)
                    *dndIndex = j;
                return &_fars[i];
            }
        }
    }

    return NULL;
}

bool Holder::setHook(bool value)
{
    if (value)
    {
        TRACE("Setting the hook\n");
        return Hook::instance()->setGetMsgProcHook();
    }
    else
    {
        TRACE("Removing the hook\n");
        return Hook::instance()->resetGetMsgProcHook();
    }
}

bool Holder::isFarWindow(HWND hwnd)
{
    return NULL != findFar(hwnd);
}

HWND Holder::getActiveDnd(HWND hFar)
{
    FarItem* f = findFar(hFar);
    if (!f)
        return 0;
    size_t i;
    for (i = 0; i < f->dnds().size(); i++)
    {
        if (isDndWindow(f->dnds()[i]))
            return f->dnds()[i];
    }
    return 0;
}

// vim: set et ts=4 ai :

