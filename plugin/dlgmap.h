#ifndef __KARBAZOL_DRAGNDROP_2_0__DLGMAP_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLGMAP_H__

class FarDialog;

class FarDialogsMap
{
private:
    struct Entry
    {
        Entry* next;
        FarDialog* dialog;
        void* handle;
    };
    Entry _head;
    FarDialogsMap(): _head(0){}
    ~FarDialogsMap();
    static void kill(FarDialogsMap* p);
public:
    static FarDialogsMap* instance();

    FarDialog* getByHandle(void* handle);
    FarDialog* register(void* handle, FarDialog* dlg);
    void unregister(void* handle);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLGMAP_H__

