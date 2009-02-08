#ifndef __KARBAZOL_DRAGNDROP_2_0__THRDFLTR_H__
#define __KARBAZOL_DRAGNDROP_2_0__THRDFLTR_H__

#include <windows.h>

/**
 * Per-thread singleton. Used by Hook singleton to filter per-thread messages.
 */
class ThreadFilter
{
private:
    bool _noDragging;
    bool _draggingChecked;
    int _dragCheckCounter;
    void reset();
    void checkDragging(HWND hwnd);
    void checkDndUnderMouse(HWND msgHwnd);
public:
    ThreadFilter(): _noDragging(false), _draggingChecked(false),
        _dragCheckCounter(1){}
    ~ThreadFilter(){}

    /**
     * Handles a system message
     */
    void handle(MSG& msg);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__THRDFLTR_H__

