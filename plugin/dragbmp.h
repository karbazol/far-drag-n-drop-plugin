#ifndef __KARBAZOL_DRAGNDROP__DRAGBMP_H__
#define __KARBAZOL_DRAGNDROP__DRAGBMP_H__

#include <windows.h>

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

