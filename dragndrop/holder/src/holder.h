/**
 * @file holder.h
 * The file contains declaration of Holder class.
 *
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__HOLDER_H__
#define __KARBAZOL_DRAGNDROP_2_0__HOLDER_H__

#include "hldrwnd.h"
#include "faritem.h"
#include <hook/hldrapi.h>

/**
 * Array of FarItem objects
 */
typedef GrowOnlyArray<FarItem> FarItems;

/**
 * Represents an instance of the holder process
 */
class Holder: public IHolder
{
private:
    HolderWindow _window;
    HANDLE _mutex;
    FarItems _fars;
    HANDLE _leftEvent;
    HANDLE _rightEvent;
    HHOOK _llMouse;
    bool _hookIsSet;
    static Holder* _instance;
    static LRESULT CALLBACK mouseHook(int,WPARAM,LPARAM);
    bool setupCurDir();
    bool createMutex();
    void validateFarItems();
private:
    // IHolder interface implementation
    const wchar_t* getHolderWindowClassName();
    const wchar_t* getHolderFileName();
    const wchar_t* getHolderMutexName();
    HANDLE getLeftButtonEvent();
    HANDLE getRightButtonEvent();
public:
    Holder();
    ~Holder();

    bool initialize();
    int run();

    void registerDND(HWND hFar, HWND dnd);
    void unregisterDND(HWND dnd);
    bool setHook(bool value);
    bool isFarWindow(HWND hwnd);
    HWND getActiveDnd(HWND hFar);

    FarItem* findFar(HWND hFar, bool append=false);
    FarItem* findDnd(HWND dnd, HWND hFar=NULL, size_t* dndIndex=NULL);
    inline size_t farsCount() const {return _fars.size();}

    void notifyOtherHolder(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HOLDER_H__

