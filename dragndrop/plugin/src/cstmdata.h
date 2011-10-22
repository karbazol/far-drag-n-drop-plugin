/**
 * @file cstmdata.h
 * Contains declaration of CustomData structure
 *
 * $Id$
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

