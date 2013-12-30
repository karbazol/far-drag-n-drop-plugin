/**
 * @file hldrintf.h
 * Contains declaration of IHolder interface used by HolderApi class
 *
 * $Id: hldrintf.h 77 2011-10-22 20:25:46Z Karbazol $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRINTF_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRINTF_H__

#include <windows.h>

/**
 * @brief Strings collection identifying holder process.
 * 
 * IHolder interface provides API to get holder process
 * related strings such as holder window class name,
 * holder executable name, and holder mutex name
 *
 * @todo Move getXXXXButtonEvent() functions to different
 * interface
 */
struct IHolder
{
    virtual const wchar_t* getHolderWindowClassName() = 0;
    virtual const wchar_t* getHolderFileName() = 0;
    virtual const wchar_t* getHolderMutexName() = 0;
    virtual HANDLE getLeftButtonEvent() = 0;
    virtual HANDLE getRightButtonEvent() = 0;
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRINTF_H__

// vim: set et ts=4 sw=4 ai :

