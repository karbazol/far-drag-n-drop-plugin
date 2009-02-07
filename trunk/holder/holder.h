/**
 * @file holder.h
 * The file contains declaration of Holder class.
 *
 * $Id$
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__HOLDER_H__
#define __KARBAZOL_DRAGNDROP_2_0__HOLDER_H__

#include "hldrwnd.h"
#include "faritem.h"

/**
 * Array of FarItem objects
 */
typedef GrowOnlyArray<FarItem> FarItems;

/**
 * Represents an instance of the holder process
 */
class Holder 
{
private:
    HolderWindow _window;
    HANDLE _mutex;
    FarItems _fars;
    HANDLE _leftEvent;
    HANDLE _rightEvent;
    HHOOK _llMouse;
    static Holder* _instance;
    static LRESULT CALLBACK mouseHook(int,WPARAM,LPARAM);
    bool setupCurDir();
    bool createMutex();
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
    FarItem* findDnd(HWND dnd, size_t* dndIndex=NULL);
    inline size_t farsCount() const {return _fars.size();}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HOLDER_H__

