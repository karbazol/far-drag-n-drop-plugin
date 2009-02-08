#include "dragbmp.h"
#include "utils.h"

DragBitmap::DragBitmap():_bmp(NULL)
{
    HDC desktop = GetDC(NULL);
    if (!desktop)
        return;

    HDC dc = CreateCompatibleDC(desktop);
    if (dc)
    {
        RECT rect = {0};

        /** @todo Draw file names during dragging instead of 'Hello world' */
        if (DrawText(dc, L"Hello world!", -1, &rect, DT_CALCRECT))
        {
            _bmp = CreateCompatibleBitmap(desktop, rect.right, rect.bottom);
            if (_bmp)
            {
                HBITMAP old = (HBITMAP)SelectObject(dc, _bmp);

                //COLORREF oldBG = SetBkColor(dc, RGB(0,0,255));
                DrawText(dc, L"Hello world!", -1, &rect, 0);
                //SetBkColor(dc, oldBG);

                SelectObject(dc, old);

                _size.cx = rect.right;
                _size.cy = rect.bottom;
            }
        }
        else
        {
            TRACE("DrawText failed\n");
        }

        DeleteDC(dc);
    }

    ReleaseDC(NULL, desktop);
}

DragBitmap::~DragBitmap()
{
    DeleteObject(_bmp);
}

HBITMAP DragBitmap::handle()
{
    return _bmp;
}

void DragBitmap::getSize(SIZE& s)
{
    s = _size;
}

// vim: set et ts=4 ai :

