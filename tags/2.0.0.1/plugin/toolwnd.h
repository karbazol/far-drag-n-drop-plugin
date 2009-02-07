#ifndef __KARBAZOL_DRAGNDROP_2_0__TOOLWND_H__
#define __KARBAZOL_DRAGNDROP_2_0__TOOLWND_H__

#include <shlobj.h>
#include "mywnd.h"
#include "myshptr.h"
#include "myunknown.h"

class ToolWindow : public MyWindow, public Unknown, public IDropSource, public IDropTarget
{
private:
    ShPtr<IDataObject> _data;
    ShPtr<IDataObject> _dropData;
    ShPtr<IDropTargetHelper> _dropHelper;
    int _mouseCounter;
    void keyStateToEffect(DWORD keyState, DWORD& effect);
protected:
    LRESULT handle(UINT msg, WPARAM wParam, LPARAM lParam);
    WCHAR* getClassName();
    void beforeCreation(DWORD& style, DWORD& styleEx);

    // Message handlers
    LRESULT prepareForDragging(IDataObject* data);
    LRESULT onMouse(UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT onDblClick(UINT msg, WPARAM wParam, LPARAM lParam);
    HWND onGetActiveFar();
    LRESULT onIsActiveFar();
    LRESULT onIsDragging();
    void onDestroy();
    void onCreate();
public:
    // Interface IUnknown
    DEFINE_UNKNOWN
    HRESULT WINAPI QueryInterface(REFIID iid, void** obj);
    // Interface IDropSource
    HRESULT WINAPI QueryContinueDrag(BOOL fEscapePressed, DWORD keyState);
    HRESULT WINAPI GiveFeedback(DWORD dwEffect);

    // Interface IDropTarget
    HRESULT WINAPI DragEnter(IDataObject* obj, DWORD keyState, POINTL pt, DWORD* effect);
    HRESULT WINAPI DragOver(DWORD keyState, POINTL pt, DWORD* effect);
    HRESULT WINAPI DragLeave();
    HRESULT WINAPI Drop(IDataObject* obj, DWORD keyState, POINTL pt, DWORD* effect);
public:
    ToolWindow();
    ~ToolWindow();
    bool show();
    bool hide();
};

#endif // __KARBAZOL_DRAGNDROP_2_0__TOOLWND_H__
// vim: set et ts=4 ai :

