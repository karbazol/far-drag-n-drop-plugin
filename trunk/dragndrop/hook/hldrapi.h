/**
 * @file hldrapi.h
 * Contains declaration of HolderApi class
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__

#include "hldrdefs.h"
#include "hook.h"

/**
 * @brief Holder API
 *
 * Class provides entry points for Holder API
 */
class HolderApi
{
private:
    HANDLE _leftEvent;
    HANDLE _rightEvent;
    HWND window();
    HolderApi();
    virtual ~HolderApi();
    static void kill(HolderApi* p);
public:
    static _HOOK_SPEC HolderApi* instance();

    virtual LRESULT windowsCreated(HWND far, HWND dnd);
    virtual LRESULT windowsDestroy(HWND dnd);
    virtual LRESULT setHook(bool value);

    // Returns far window on success otherwise NULL
    virtual HWND isFarWindow(HWND hwnd);

    // Returns dragndrop window on success otherwise NULL
    virtual HWND getActiveDnd(HWND hFar);

    virtual bool isLeftButtonDown() const;
    virtual bool isRightButtonDown()const;
};

void makeSureHolderRun(const wchar_t* holderMutex,
        const wchar_t* holderExecutable);
#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__

