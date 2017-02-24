
#include <common/utils.h>
#include <dll/dll.h>
#include <dll/mystring.h>
#include <hook/hldrapi.h>
#include <hook/hook.h>

#include "thrdfltr.h"

class HookImpl: public Hook
{
    friend static Hook* Hook::instance();

    HHOOK _hook;
    DWORD _tlsId;

    HookImpl():_hook(0),_tlsId(0)
    {
        TRACE("Hook loaded into the process\n");
    }

    ~HookImpl()
    {
        TRACE("Hook unloading from the process\n");
        resetGetMsgProcHook();
    }

    ThreadFilter* filter()
    {
        ThreadFilter* f = reinterpret_cast<ThreadFilter*>(TlsGetValue(_tlsId));
        if (!f)
        {
            f = new ThreadFilter();
            TlsSetValue(_tlsId, f);
        }

        return f;
    }

    static void thrdEnd(intptr_t tlsId)
    {
        if (tlsId)
        {
            ThreadFilter* f = reinterpret_cast<ThreadFilter*>(TlsGetValue(static_cast<DWORD>(tlsId)));
            if (f)
            {
                TlsSetValue(static_cast<DWORD>(tlsId), 0);
                delete f;
            }
        }
    }

    static void kill(HookImpl* p)
    {
        if (p)
        {
            thrdEnd(p->_tlsId);
            TlsFree(p->_tlsId);
            delete p;
        }
    }

    bool setGetMsgProcHook()
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

    bool resetGetMsgProcHook()
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

    static LRESULT CALLBACK getMsgProc(int code, WPARAM wParam, LPARAM lParam)
    {
        if (HC_ACTION == code && wParam == PM_REMOVE)
        {
            HookImpl* This = instance();
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

    static HookImpl* instance()
    {
        static HookImpl* p = 0;
        if (!p)
        {
            p = new HookImpl();
            if (p)
            {
                p->_tlsId = TlsAlloc();

                Dll* dll = Dll::instance();
                if (dll)
                {
                    dll->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
                    dll->registerThreadEndCallBack(reinterpret_cast<PdllCallBack>(&thrdEnd), (void*)(intptr_t)p->_tlsId);
                }
            }
        }

        return p;
    }

}; // class HookImpl

Hook* Hook::instance()
{
    return HookImpl::instance();
}

// vim: set et ts=4 ai :

