/**
 * @file dll_utils.h
 * The file contains declaration of utility classes and routines used by DLLs.
 *
 * $Id: dll_utils.h 76 2008-10-04 07:21:38Z eleskine $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DLL_UTILS_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLL_UTILS_H__

#include "mystring.h"

MyStringW getErrorDesc(unsigned int e);
MyStringW getLastErrorDesc();

#endif // __KARBAZOL_DRAGNDROP_2_0__DLL_UTILS_H__

// vim: set et ts=4 sw=4 ai :

