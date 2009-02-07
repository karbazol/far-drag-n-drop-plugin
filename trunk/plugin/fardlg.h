// $Id$

/**
 * @file fardlg.h
 * Contains declarationa of FarDialog class.
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__

#include "far.h"

/**
 * Structure used to initialize dialog items
 */
struct InitDialogItem
{
    int Type;
    int X1;
    int Y1;
    int X2;
    int Y2;
    int Focus;
    int Selected;
    unsigned int Flags;
    int DefaultButton;
    char *Data;
};

class DialogShower;

/**
 * Use this class as base for your dialogs.
 */
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
    virtual InitDialogItem* items();
    virtual DWORD flags();
protected:
    // HANDLERS
    virtual bool onInit();
    virtual bool onClose(int closeId);
    bool lock();
    void unlock();
public:
    FarDialog();
    virtual ~FarDialog();
   
    /**
     * Allows to get Far dialog handle.
     * @return value of underlaying Far dialog handle.
     */
    inline HANDLE hwnd(){return _hwnd;}

    /**
     * Allows to determine whether the dialog is running
     */
    inline bool running(){return _running;}

    /**
     * Override this function to allow Dialog framework to show the dialog.
     */
    virtual int itemsCount();

    int show(bool modal);
    int hide();
    
    long sendMessage(int msg, int param1, long param2);
    void postMessage(int msg, int param1, long param2);

    // Operations with the dialog controls
    bool enable(int id);
    bool disable(int id);
    int switchCheckBox(int id, int state);
    int checkState(int id);
    inline bool checked(int id)
    {
        return checkState(id) != BSTATE_UNCHECKED;
    }
    friend class DialogShower;
};

#ifndef getFarDlgItemId

/**
 * Utility macro to determine ID of Far dialog item
 */
#define getFarDlgItemId(structure,item)         \
((size_t)(&((structure*)0)->item)/sizeof(InitDialogItem))
#endif

#endif // __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__
// vim: set et ts=4 ai :

