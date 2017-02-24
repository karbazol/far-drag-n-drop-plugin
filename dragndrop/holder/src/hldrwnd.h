#ifndef __KARBAZOL_DRAGNDROP_2_0__HLDRWND_H__
#define __KARBAZOL_DRAGNDROP_2_0__HLDRWND_H__

#include <common/mywnd.h>
#include <common/hldrdefs.h>

class Holder;

/**
 * The main window of HOLDER.DND process
 */
class HolderWindow : public MyWindow
{
private:
    Holder* _holder;
protected:
    WCHAR* getClassName();
    LRESULT handle(UINT msg, WPARAM wParam, LPARAM lParam);
private:
    // Message handlers
    LRESULT onFarWindowsCreated(HWND far, HWND dnd);
    LRESULT onDndWindowDestroy(HWND dnd);
    LRESULT onIsDndWindow(HWND dnd);
    LRESULT onIsFarWindow(HWND far);
    LRESULT onGetDndWindow(HWND far);
    LRESULT onGetFarWindow(HWND dnd);
    LRESULT onSetTheHook(bool value);
public:
    HolderWindow(): MyWindow(), _holder(NULL) {}
    ~HolderWindow(){}
    inline void holder(Holder* value){_holder = value;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HLDRWND_H__

