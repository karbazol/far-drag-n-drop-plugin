
#include <common/utils.h>
#include <hook/dndcmnct.h>
#include <hook/hook.h>

#include "holder.h"

Holder::Holder(): _window(), _mutex(0), _fars(), _hookIsSet(false)
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
    HolderApi* holderApi = HolderApi::instance();

    if (holderApi)
    {
        holderApi->setHolder(NULL);
    }

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

const wchar_t* Holder::getHolderWindowClassName()
{
    return HOLDER_OTHER_CLASS_NAME;
}

const wchar_t* Holder::getHolderFileName()
{
    return HOLDER_OTHER_EXECUTABLE;
}

const wchar_t* Holder::getHolderMutexName()
{
    return HOLDER_OTHER_MUTEX;
}

HANDLE Holder::getLeftButtonEvent()
{
    return _leftEvent;
}

HANDLE Holder::getRightButtonEvent()
{
    return _rightEvent;
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
    {
        return false;
    }

    if (!createMutex())
    {
        return false;
    }

    if (!_window.create(NULL))
    {
        return false;
    }

    HolderApi* holderApi = HolderApi::instance();

    if (holderApi)
    {
        holderApi->setHolder(this);
    }

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

void Holder::validateFarItems()
{
    size_t i, j;

    TRACE("Entering validateFarItems. _far.size() == %d\n", _fars.size());

    for (i = 0; i < _fars.size(); ++i)
    {
        while (!IsWindow(_fars[i].hwnd()))
        {
            if (i >= _fars.deleteItem(i))
            {
                TRACE("Leaving validateFarItems. _far.size() == %d\n", _fars.size());
                return;
            }
        }

        HwndsArray& dnds = _fars[i].dnds();
        for (j = 0; j < dnds.size(); ++j)
        {
            while (!IsWindow(dnds[j]))
            {
                if (j >= dnds.deleteItem(j))
                {
                    break;
                }
            }
        }

        if (dnds.size() == 0)
        {
            _fars.deleteItem(i);
        }
    }
    TRACE("Leaving validateFarItems. _far.size() == %d\n", _fars.size());
}

void Holder::registerDND(HWND hFar, HWND dnd)
{
    validateFarItems();

    size_t dndIndex = 0;
    FarItem* item = findDnd(dnd, hFar, &dndIndex);

    if (!item)
    {
        item = &_fars.append(hFar);
    }

    ASSERT(item && item->hwnd() == hFar);

    if (dndIndex == item->dnds().size())
    {
        item->dnds().append(dnd);

        HolderApi* holderApi = HolderApi::instance();
        if (holderApi)
        {
            holderApi->windowsCreated(hFar, dnd);
        }
    }
}

void Holder::unregisterDND(HWND dnd)
{
    size_t dndIndex;
    FarItem* item = findDnd(dnd, NULL, &dndIndex);

    if (item)
    {
        ASSERT(dndIndex < item->dnds().size());

        if (!item->dnds().deleteItem(dndIndex))
        {
            _fars.deleteItem(item - &_fars[0]);
        }

        HolderApi* holderApi = HolderApi::instance();

        if (holderApi)
        {
            holderApi->windowsDestroy(dnd);
        }
    }

    validateFarItems();
}

FarItem* Holder::findFar(HWND hFar, bool append)
{
    size_t i;

    for (i = 0; i < _fars.size(); i++)
    {
        if (_fars[i].hwnd() == hFar)
        {
            return &_fars[i];
        }
    }

    if (append)
    {
        return &_fars.append(hFar);
    }

    return NULL;
}

FarItem* Holder::findDnd(HWND dnd, HWND hFar, size_t* dndIndex)
{
    size_t i;

    if (dndIndex)
    {
        *dndIndex = 0;
    }

    if (hFar)
    {
        FarItem* item = findFar(hFar, false);

        if (item)
        {
            for (i = 0; i <item->dnds().size(); i++)
            {
                if (item->dnds()[i] == dnd)
                {
                    if (dndIndex)
                    {
                        *dndIndex = i;
                    }

                    return item;
                }
            }

            if (dndIndex)
            {
                *dndIndex = i;
                return item;
            }
        }

        return NULL;
    }

    for (i = 0; i < _fars.size(); i++)
    {
        size_t j;
        for (j = 0; j < _fars[i].dnds().size(); j++)
        {
            if (_fars[i].dnds()[j] == dnd)
            {
                if (dndIndex)
                {
                    *dndIndex = j;
                }
                return &_fars[i];
            }
        }
    }

    return NULL;
}

bool Holder::setHook(bool value)
{
    HolderApi* holderApi = HolderApi::instance();

    if (holderApi)
    {
        holderApi->setHook(value);
    }

    Hook* theHook = Hook::instance();
    if (!theHook)
    {
        return false;
    }

    if (value)
    {
        if (!_hookIsSet)
        {
            TRACE("Setting the hook\n");
            _hookIsSet = theHook->setGetMsgProcHook();

            return _hookIsSet;
        }

    }
    else
    {
        if (_hookIsSet)
        {
            TRACE("Removing the hook\n");
            _hookIsSet = !theHook->resetGetMsgProcHook();

            return !_hookIsSet;
        }

    }

    return false;
}

bool Holder::isFarWindow(HWND hwnd)
{
    return NULL != findFar(hwnd);
}

HWND Holder::getActiveDnd(HWND hFar)
{
    FarItem* f = findFar(hFar);
    if (!f)
    {
        return 0;
    }
    size_t i;
    for (i = 0; i < f->dnds().size(); i++)
    {
        if (isDndWindow(f->dnds()[i]))
        {
            return f->dnds()[i];
        }
    }
    return 0;
}

// vim: set et ts=4 ai :

