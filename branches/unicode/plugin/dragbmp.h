/**
 * @file dragbmp.h
 * The file contains declaration of DragBitmap class.
 *
 * $Id: dragbmp.h 44 2008-05-11 14:22:01Z eleskine $
 */

#ifndef __KARBAZOL_DRAGNDROP__DRAGBMP_H__
#define __KARBAZOL_DRAGNDROP__DRAGBMP_H__

#include <windows.h>

/**
 * Represents a bitmap to be shown during dragging operation.
 */
class DragBitmap
{
private:
    HBITMAP _bmp;
    SIZE _size;
public:
    DragBitmap();
    ~DragBitmap();
    HBITMAP handle();
    void getSize(SIZE&);
};

#endif // __KARBAZOL_DRAGNDROP__DRAGBMP_H__

