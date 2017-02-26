/**
 * @file fardlg.h
 * Contains declarationa of FarDialog class.
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__FARDLG_H__

#include <ddlock.h>
#include <refcounted.hpp>
#include "far.h"

/**
 * Structure used to initialize dialog items
 */
struct InitDialogItem
{
    enum FARDIALOGITEMTYPES Type;
    int X1;
    int Y1;
    int X2;
    int Y2;
    int Focus;
    intptr_t Selected;
    unsigned int Flags;
    int DefaultButton;
    const wchar_t *Data;
};

class DialogShower;

/**
 * Use this class as base for your dialogs.
 */
class FarDialog: public RefCounted
{
private:
    volatile HANDLE _hwnd;
    CriticalSection _textGuard;
    MyStringW** volatile _controlTexts;
    static intptr_t WINAPI dlgProc(HANDLE dlg, intptr_t msg, intptr_t param1, void* param2);
    intptr_t doShow();
    intptr_t run(void*& farItems);
    void restoreItems();
    void freeFarItems(void* farItems);
protected:
    virtual intptr_t handle(intptr_t msg, intptr_t param1, void* param2);
    virtual const GUID& Id() const = 0;
    virtual int left();
    virtual int top();
    virtual int right();
    virtual int bottom();
    virtual const wchar_t* help();
    virtual void prepareItems(int consoleWidth, int consoleHeight) {consoleWidth; consoleHeight;}
    virtual InitDialogItem* items();
    virtual void releaseItems() {}
    virtual DWORD flags();
protected:

    // HANDLERS
    virtual bool onInit();
    virtual bool onClose(intptr_t closeId);
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
    inline bool running(){return !!_hwnd;}

    /**
     * Override this function to allow Dialog framework to show the dialog.
     */
    virtual size_t itemsCount();

    intptr_t show(bool modal);
    intptr_t hide();

    intptr_t sendMessage(intptr_t msg, intptr_t param1, void* param2);

    // Operations with the dialog controls
    intptr_t setText(intptr_t id, const wchar_t* value);
    bool enable(intptr_t id);
    bool disable(intptr_t id);
    intptr_t switchCheckBox(intptr_t id, intptr_t state);
    intptr_t checkState(intptr_t id);
    bool checked(intptr_t id);
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

