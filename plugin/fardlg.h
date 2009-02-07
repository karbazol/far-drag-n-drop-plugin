#ifndef __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__

#include "far.h"

class FarDialog
{
private:
    HANDLE volatile _hwnd;
    bool volatile _running;
    static long WINAPI dlgProc(HANDLE dlg, int msg, int param1, long param2);
    int doShow();
protected:
    virtual long handle(int msg, int param1, long param2);
    virtual int left();
    virtual int top();
    virtual int right();
    virtual int bottom();
    virtual const char* help();
    virtual InitDialogItem* items() = 0;
    virtual DWORD flags();
protected:
    // HANDLERS
    virtual bool onInit();
    virtual bool onClose(int closeId);
public:
    FarDialog();
    virtual ~FarDialog();
    
    inline HANDLE hwnd(){return _hwnd;}
    inline bool running(){return _running;}
    virtual int itemsCount() = 0;

    int show(bool modal);
    int hide();
    
    long sendMessage(int msg, int param1, long param2);
    void postMessage(int msg, int param1, long param2);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__
// vim: set et ts=4 ai :

