/**
 * @file protect.h
 * Contains declaration of MemProtect class.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__PROTECT_H__
#define __KARBAZOL_DRAGNDROP_2_0__PROTECT_H__

#include <windows.h>

/**
 * @brief Memory region protection modifier.
 *
 * Utility class used to temporaly modify protection mode of specified memory region.
 */
class MemProtect
{
private:
    MEMORY_BASIC_INFORMATION _mi;
    DWORD _oldProtect;
public:
    MemProtect(void* p, DWORD protect);
    ~MemProtect();
};

#endif // __KARBAZOL_DRAGNDROP_2_0__PROTECT_H__

// vim: set et ts=4 :

