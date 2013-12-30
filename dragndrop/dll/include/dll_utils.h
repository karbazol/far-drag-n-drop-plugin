/**
 * @file dll_utils.h
 * The file contains declaration of utility classes and routines used by DLLs.
 *
 * $Id: dll_utils.h 77 2011-10-22 20:25:46Z Karbazol $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DLL_UTILS_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLL_UTILS_H__

#include <common/utils.h>
#include "mystring.h"

MyStringW getErrorDesc(unsigned int e);
MyStringW getLastErrorDesc();

#endif // __KARBAZOL_DRAGNDROP_2_0__DLL_UTILS_H__

// vim: set et ts=4 sw=4 ai :

