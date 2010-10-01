#include <shlwapi.h>
#include "dndmsgs.h"
#include "hldrapi.h"
#include "utils.h"
#include "dll.h"
#include "mystring.h"

static bool checkMutex()
{
    HANDLE mutex = OpenMutex(MUTEX_MODIFY_STATE, FALSE, HOLDER_MUTEX);
    if (mutex)
    {
        CloseHandle(mutex);
        return true;
    }

    if (GetLastError() != ERROR_FILE_NOT_FOUND)
        return true;

    return false;
}

static MyStringW getHolderFileName()
{
    size_t chars = MAX_PATH;
    MyStringW res;
    res.length(chars);

    while (chars == 
            GetModuleFileName(
                reinterpret_cast<HMODULE>(getMyModuleBaseAddress()), res, 
                static_cast<DWORD>(chars)))
    {
        chars += MAX_PATH;

        res.length(chars);
    }

    WCHAR* file = StrRChr(res, NULL, L'\\');

    if (!file)
    {
        res.length(0);
        return res;
    }

    res.length(file - res);

    res /= L"holder.dnd";

    return res;
}

static bool runHolder()
{
    MyStringW holder = getHolderFileName();

    if (!holder)
        return false;

    STARTUPINFO si = {sizeof(si), 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sec={
        sizeof(sec),
        getEveryOneDescriptor(),
        FALSE
    };

    bool res = false;

    if (CreateProcess(NULL, holder, &sec, &sec, FALSE, 0, NULL, NULL, &si, &pi))
    {
        res = true;
        WaitForInputIdle(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    freeEveryOneDescriptor(sec.lpSecurityDescriptor);

    return res;
}

static void makeSureHolderRun()
{
    if (!checkMutex())
        runHolder();
}

HolderApi::HolderApi()
{
    makeSureHolderRun();
    _leftEvent = OpenEvent(SYNCHRONIZE, FALSE, HOLDER_LEFT_EVENT);
    _rightEvent = OpenEvent(SYNCHRONIZE, FALSE, HOLDER_RIGHT_EVENT);
}

HolderApi::~HolderApi()
{
    CloseHandle(_leftEvent);
    CloseHandle(_rightEvent);
}

HolderApi* HolderApi::instance()
{
    static HolderApi* p = 0;

    if (!p)
    {
        p = new HolderApi;
        Dll::instance()->registerProcessEndCallBack((PdllCallBack)&kill, p);
    }

    return p;
}

void HolderApi::kill(HolderApi* p)
{
    if (p)
    {
        delete p;
    }
}

HWND HolderApi::window()
{
    static HWND hldr = NULL;
    if (HOLDER_YES == SendMessage(hldr, WM_ARE_YOU_HOLDER, 0, 0))
        return hldr;

    makeSureHolderRun();

    hldr = FindWindow(HOLDER_CLASS_NAME, NULL);
    if (HOLDER_YES != SendMessage(hldr, WM_ARE_YOU_HOLDER, 0, 0))
        hldr = NULL;

    return hldr;
}

LRESULT HolderApi::windowsCreated(HWND f, HWND dnd)
{
    HWND hldr = window();
    if (hldr)
        return SendMessage(hldr, WM_FARWINDOWSCREATED, 
                reinterpret_cast<WPARAM>(f), reinterpret_cast<LPARAM>(dnd));
    return HOLDER_NOT;
}

LRESULT HolderApi::windowsDestroy(HWND dnd)
{
    HWND hldr = window();
    if (hldr)
        return SendMessage(hldr, WM_DNDWINDOWDESTROY, reinterpret_cast<WPARAM>(dnd), 0);
    return HOLDER_NOT;
}

LRESULT HolderApi::setHook(bool value)
{
    HWND hldr = window();
    if (hldr)
        return SendMessage(hldr, WM_HLDR_SETHOOK, value?1:0, 0);
    return HOLDER_NOT;
}

HWND HolderApi::isFarWindow(HWND hwnd)
{
    HWND hldr = window();
    if (!hldr)
        return NULL;

    //
    // The wfx2far adds a child window to far window and when a mouse is over it we
    // should find the window which has no parent and it will be far window.
    //
    for(HWND hParent; NULL != (hParent = GetParent(hwnd)); hwnd = hParent);
    
    if (HOLDER_YES == SendMessage(hldr, WM_HLDR_ISFARWND, reinterpret_cast<WPARAM>(hwnd), 0))
        return hwnd;

    return NULL;
}

HWND HolderApi::getActiveDnd(HWND hFar)
{
    HWND res = window();
    if (!res)
        return NULL;

    return reinterpret_cast<HWND>(
            SendMessage(res, WM_HLDR_GETDNDWND, reinterpret_cast<WPARAM>(hFar), 0));
}

// vim: set et ts=4 ai :

