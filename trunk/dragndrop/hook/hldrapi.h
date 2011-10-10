/**
 * @file hldrapi.h
 * Contains declaration of HolderApi class
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__

#include "hldrdefs.h"
#include "hldrintf.h"
#include "hook.h"

/**
 * @brief Holder API
 *
 * Class provides entry points for Holder API
 */
struct HolderApi
{
    static _HOOK_SPEC HolderApi* instance();

    virtual bool setHolder(IHolder* holder) = 0;

    virtual LRESULT windowsCreated(HWND far, HWND dnd) = 0;
    virtual LRESULT windowsDestroy(HWND dnd) = 0;
    virtual LRESULT setHook(bool value) = 0;

    // Returns far window on success otherwise NULL
    virtual HWND isFarWindow(HWND hwnd) = 0;

    // Returns dragndrop window on success otherwise NULL
    virtual HWND getActiveDnd(HWND hFar) = 0;

    virtual bool isLeftButtonDown() const = 0;
    virtual bool isRightButtonDown()const = 0;
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__

