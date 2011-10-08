#include "hldrapi.h"
#include "hook.h"
#include "utils.h"
#include "dll.h"
#include "thrdfltr.h"
#include "mystring.h"

void Hook::makeSureHolderRun(const wchar_t* holderMutex, const wchar_t* holderExecutable)
{
    ::makeSureHolderRun(holderMutex, holderExecutable);
}


Hook::~Hook()
{
    resetGetMsgProcHook();
}

Hook* Hook::instance()
{
    static Hook* p = 0;
    if (!p)
    {
        p = new Hook();
        if (p)
        {
            p->_tlsId = TlsAlloc();

            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
                dll->registerThreadEndCallBack(reinterpret_cast<PdllCallBack>(&thrdEnd), p);
            }
        }
    }

    return p;
}

ThreadFilter* Hook::filter()
{
    ThreadFilter* f = reinterpret_cast<ThreadFilter*>(TlsGetValue(_tlsId));
    if (!f)
    {
        f = new ThreadFilter;
        TlsSetValue(_tlsId, f);
    }

    return f;
}

void Hook::thrdEnd(Hook* p)
{
    if (p)
    {
        ThreadFilter* f = reinterpret_cast<ThreadFilter*>(TlsGetValue(p->_tlsId));
        if (f)
        {
            delete f;
            TlsSetValue(p->_tlsId, 0);
        }
    }
}

void Hook::kill(Hook* p)
{
    if (p)
    {
        thrdEnd(p);
        TlsFree(p->_tlsId);
        delete p;
    }
}

bool Hook::setGetMsgProcHook()
{
    if (_hook)
    {
        return false;
    }

    _hook = SetWindowsHookEx(WH_GETMESSAGE, &getMsgProc, 
            (HINSTANCE)getMyModuleBaseAddress(), 0);

#if _DEBUG
    if (!_hook)
    {
        TRACE("Failed to set the hook\n");
        LASTERROR();
    }
    else
    {
        TRACE("The hook is set\n");
    }
#endif

    return _hook?true:false;
}

bool Hook::resetGetMsgProcHook()
{
    if (!_hook)
        return false;
    TRACE("Unhooking the hook\n");
    if (UnhookWindowsHookEx(_hook))
    {
        // Provoke apps to call GetMessage
        PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
        _hook = NULL;
        return true;
    }

    return false;
}

LRESULT Hook::getMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (HC_ACTION == code && wParam == PM_REMOVE)
    {
        Hook* This = instance();
        if (This)
        {
            ThreadFilter* filter = This->filter();
            if (filter)
            {
                filter->handle(*reinterpret_cast<MSG*>(lParam));
            }
        }
    }
    return CallNextHookEx((HHOOK)-1, code, wParam, lParam);
}

// vim: set et ts=4 ai :

