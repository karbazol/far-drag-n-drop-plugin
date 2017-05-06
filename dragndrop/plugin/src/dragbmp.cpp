#include <utils.h>
#include <ShObjIdl.h>
#include "dragbmp.h"

#if 0
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
    HBITMAP releaseHandle();
    void getSize(SIZE&);
};

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
    if (_bmp)
        DeleteObject(_bmp);
}

HBITMAP DragBitmap::releaseHandle()
{
    HBITMAP bmp = _bmp;
    _bmp = NULL;
    return bmp;
}

void DragBitmap::getSize(SIZE& s)
{
    s = _size;
}

void generateDragImage(SHDRAGIMAGE& img)
{
    DragBitmap theBmp;
    theBmp.getSize(img.sizeDragImage);
    img.ptOffset.x = img.ptOffset.y = 0;
    img.hbmpDragImage = theBmp.releaseHandle();
    img.crColorKey = RGB(0,0,255);
}
#endif

static void detectIconParams(HICON hIcon, HDC hTempDC, int& width, int& height, bool& hasAlpha)
{
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);
    BITMAPINFOHEADER bmi = { sizeof(bmi) };
    // default values
    hasAlpha = false;
    width = GetSystemMetrics(SM_CXICON);
    height = GetSystemMetrics(SM_CYICON);
    if (!iconInfo.hbmColor)
    {
        // black-white icon
        if (GetDIBits(hTempDC, iconInfo.hbmMask, 0, 0, NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS))
        {
            width = bmi.biWidth;
            height = bmi.biHeight;
        }
        DeleteObject(iconInfo.hbmMask);
        return;
    }
    if (GetDIBits(hTempDC, iconInfo.hbmColor, 0, 0, NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS))
    {
        width = bmi.biWidth;
        height = bmi.biHeight;
        bmi.biBitCount = 32;
        bmi.biCompression = BI_RGB;
        size_t numPixels = bmi.biWidth * bmi.biHeight;
        DWORD* bits = (DWORD*)malloc(numPixels * 4);
        if (bits)
        {
            if (GetDIBits(hTempDC, iconInfo.hbmColor, 0, bmi.biHeight, bits, (BITMAPINFO*)&bmi, DIB_RGB_COLORS))
            {
                bool foundAlpha = false;
                for (size_t i = 0; !foundAlpha && i < numPixels; i++)
                    if (bits[i] >> 24)
                        foundAlpha = true;
                hasAlpha = foundAlpha;
            }
            free(bits);
        }
    }
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
}

bool generateDragImage(const MyStringW& theDragFile, SHDRAGIMAGE& img)
{
    HICON hIcon;
    if (theDragFile.length())
    {
        SHFILEINFO sfi;
        DWORD_PTR ret = SHGetFileInfo(theDragFile, 0, &sfi, sizeof(sfi), SHGFI_ICON);
        hIcon = ret ? sfi.hIcon : NULL;
    }
    else
    {
        /*
            For multiple files, reuse the generic icon from shell32 dialog
            displaying properties of multiple files.
            It is not documented and not exposed to programs directly,
            but can be reached as icon #133 in shell32.dll.
            MS can theoretically change this, but given that
            a) it is the same in XP, Win7, Win10, and
            b) we have an alternative code for Vista+ and all future systems,
            this is unlikely to cause any trouble for us.
        */
        // we use static binding to shell32!SHGetFileInfo anyway,
        // so we don't need to worry about shell32 presence
        HMODULE hShell32 = GetModuleHandle(TEXT("shell32.dll"));
        hIcon = (HICON)LoadImage(hShell32, MAKEINTRESOURCE(133), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    }
    if (!hIcon)
        return false;
    HDC hdc = CreateCompatibleDC(NULL);
    int cx, cy;
    bool hasAlpha;
    detectIconParams(hIcon, hdc, cx, cy, hasAlpha);
    img.sizeDragImage.cx = cx;
    img.sizeDragImage.cy = cy;
    img.crColorKey = hasAlpha ? CLR_NONE : RGB(0, 0, 0);
    img.hbmpDragImage = CreateBitmap(cx, cy, 1, 32, NULL);
    HGDIOBJ hbmPrev = SelectObject(hdc, img.hbmpDragImage);
    DrawIconEx(hdc, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);
    SelectObject(hdc, hbmPrev);
    DeleteDC(hdc);
    DestroyIcon(hIcon);
    img.ptOffset.x = img.sizeDragImage.cx / 2;
    img.ptOffset.y = img.sizeDragImage.cy;
    return true;
}

// vim: set et ts=4 ai :

