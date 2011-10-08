/**
 * @file hldrapi.h
 * Contains declaration of HolderApi class
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__

#include "hldrdefs.h"

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
    ~HolderApi();
    static void kill(HolderApi* p);
public:
    static HolderApi* instance();

    LRESULT windowsCreated(HWND far, HWND dnd);
    LRESULT windowsDestroy(HWND dnd);
    LRESULT setHook(bool value);

    // Returns far window on success otherwise NULL
    HWND isFarWindow(HWND hwnd);

    // Returns dragndrop window on success otherwise NULL
    HWND getActiveDnd(HWND hFar);

    inline bool isLeftButtonDown() const {return WaitForSingleObject(_leftEvent, 0) == WAIT_OBJECT_0;}
    inline bool isRightButtonDown()const {return WaitForSingleObject(_rightEvent, 0) == WAIT_OBJECT_0;}
};

void makeSureHolderRun(const wchar_t* holderMutex,
        const wchar_t* holderExecutable);
#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRAPI_H__

