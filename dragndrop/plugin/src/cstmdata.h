/**
 * @file cstmdata.h
 * Contains declaration of CustomData structure
 *
 * $Id: cstmdata.h 77 2011-10-22 20:25:46Z Karbazol $
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__CSTMDATA_H__
#define __KARBAZOL_DRAGNDROP_2_0__CSTMDATA_H__

#include <windows.h>

/**
 * @brief Custom data
 */
struct CustomData
{
    FORMATETC fmt;
    STGMEDIUM mdm;
};

#endif // __KARBAZOL_DRAGNDROP_2_0__CSTMDATA_H__

