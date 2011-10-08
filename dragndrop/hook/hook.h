#ifndef __KARBAZOL_DRAGNDROP_2_0__HOOK_H__
#define __KARBAZOL_DRAGNDROP_2_0__HOOK_H__

#include <windows.h>
#include "hldrdefs.h"

#ifdef _HOOK_EXPORTS
#define _HOOK_SPEC __declspec(dllexport)
#else
#define _HOOK_SPEC __declspec(dllimport)
#endif

class ThreadFilter;

/**
 * Singleton. Manages system hooks used by the plugin
 */
class Hook
{
private:
    HHOOK _hook;
    DWORD _tlsId;
    Hook():_hook(0),_tlsId(0){}
    ~Hook();
    static void kill(Hook* p);
    static void thrdEnd(Hook* p);
    static LRESULT CALLBACK getMsgProc(int code, WPARAM wParam, LPARAM lParam);
    ThreadFilter* filter();
public:

    /**
     * Retruns the pointer to the singleton's instance.
     */
    static _HOOK_SPEC Hook* instance();

    /**
     * Sets system-wide GetMsgProc hook.
     */
    virtual bool setGetMsgProcHook();

    /**
     * Removes system-wide GetMsgProc hook.
     */
    virtual bool resetGetMsgProcHook();

    /**
     * Function used to run specified holder_XXX.dnd process synchronized with specific mutex.
     */
    static _HOOK_SPEC void makeSureHolderRun(const wchar_t* holderMutex = HOLDER_MUTEX,
            const wchar_t* holderExecutable = HOLDER_EXECUTABLE);
};


#endif // __KARBAZOL_DRAGNDROP_2_0__HOOK_H__

// vim: set et ts=4 ai :

