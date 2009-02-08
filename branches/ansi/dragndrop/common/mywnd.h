#ifndef __KARBAZOL_2_0__MYWND_H_
#define __KARBAZOL_2_0__MYWND_H_

#include <windows.h>

/**
 * Abstract class used as common ancestor for all window classes used in the plugin.
 */
class MyWindow
{
private:
    HWND _hwnd;
    HWND _parent;
    static MyWindow* getThis(HWND hwnd);
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool createClass();
protected:

    /**
     * Implementation should return a pointer to window class name
     */
    virtual WCHAR* getClassName() = 0;

    /**
     * Initializes WNDCLASSEX structure used by Windows(R) to create the window instances.
     */
    virtual bool initClass(WNDCLASSEX& cls);

    /**
     * Called internally just before the moment when window is created.
     * Override it to set your own style and styleEx values.
     */
    virtual void beforeCreation(DWORD& style, DWORD& styleEx);

    /**
     * Called internaly by WindowProc.
     */
    virtual LRESULT handle(UINT msg, WPARAM wParam, LPARAM lParam);
public:
    MyWindow(): _hwnd(0), _parent(0){}
    virtual ~MyWindow(){}

    /**
     * Creates system window and associates it with MyWindow instance.
     */
    bool create(HWND parent);

    /**
     * Destroys system window associated with MyWindow instacne.
     */
    bool destroy();
    inline HWND hwnd(){return _hwnd;}
    inline HWND parent(){return _parent;}
    inline LRESULT sendMessage(UINT msg, WPARAM wParam=0, LPARAM lParam=0){return SendMessage(_hwnd, msg, wParam, lParam);}
    inline LRESULT postMessage(UINT msg, WPARAM wParam=0, LPARAM lParam=0){return PostMessage(_hwnd, msg, wParam, lParam);}
    inline HWND setCapture(){return SetCapture(_hwnd);}
};

#endif // __KARBAZOL_2_0__MYWND_H_
