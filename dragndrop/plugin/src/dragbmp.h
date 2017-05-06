/**
 * @file dragbmp.h
 * The file contains declaration of DragBitmap class.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP__DRAGBMP_H__
#define __KARBAZOL_DRAGNDROP__DRAGBMP_H__

#include <windows.h>
#include <mystring.h>
struct SHDRAGIMAGE;

/**
 * Generates a bitmap to be shown during dragging operation.
 *
 * @param[in] theDragFile is the full name of dragging file,
 *            or empty string for several dragging files
 * @param[out] img specifies data for IDragSourceHelper::InitializeFromBitmap
 * @return true on success, false on failure
 * @note on success, the caller is responsible for DeleteObject(img.hbmpDragImage),
 *       unless IDragSourceHelper::InitializeFromBitmap is called
 *       (InitializeFromBitmap cares about destructing it's argument)
 */
bool generateDragImage(const MyStringW& theDragFile, SHDRAGIMAGE& img);

#endif // __KARBAZOL_DRAGNDROP__DRAGBMP_H__

