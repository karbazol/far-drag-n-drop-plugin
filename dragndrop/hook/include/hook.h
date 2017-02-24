#ifndef __KARBAZOL_DRAGNDROP_2_0__HOOK_H__
#define __KARBAZOL_DRAGNDROP_2_0__HOOK_H__

#include <windows.h>
#include <common/hldrdefs.h>

#ifdef _HOOK_EXPORTS
#define _HOOK_SPEC __declspec(dllexport)
#else
#define _HOOK_SPEC __declspec(dllimport)
#endif

/**
 * Singleton. Manages system hooks used by the plugin
 */
struct Hook
{
    /**
     * Retruns the pointer to the singleton's instance.
     */
    static _HOOK_SPEC Hook* instance();

    /**
     * Sets system-wide GetMsgProc hook.
     */
    virtual bool setGetMsgProcHook() = 0;

    /**
     * Removes system-wide GetMsgProc hook.
     */
    virtual bool resetGetMsgProcHook() = 0;
};


#endif // __KARBAZOL_DRAGNDROP_2_0__HOOK_H__

// vim: set et ts=4 ai :

