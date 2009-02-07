/**
 * @file cstmdata.h
 * Contains declaration of CustomData structure
 *
 * $Id: cstmdata.h 53 2008-05-11 15:07:04Z eleskine $
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

