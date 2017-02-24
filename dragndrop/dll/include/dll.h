/**
 * @file dll.h
 * Contains declaration of Dll class
 *
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__DLL_H__
#define __KARBAZOL_DRAGNDROP_2_0__DLL_H__

#include <windows.h>
#include <common/ddlock.h>

class CallBackInfo;

/**
 * Defines a prototype for user function to be called
 * by Dll instance.
 */
typedef void (*PdllCallBack)(void* userData);

/**
 * Singleton to track thread creation/termination and process exit events
 */
class Dll
{
private:
    CriticalSection _guard;
    /**
     * Points to the linked list of routines to be called when a new
     * thread starts
     */
    CallBackInfo* _threadStart;
    /**
     * Points to the linked list of routines to be called when the 
     * current thread is about to exit
     */
    CallBackInfo* _threadEnd;
    /**
     * Points to the linked list of routines to be called when the 
     * process is about to exit
     */
    CallBackInfo* _processEnd;
    Dll():_guard(), _threadStart(0), _threadEnd(0), _processEnd(0){}
    ~Dll();
    static void endDll(void* me);

    void doCall(CallBackInfo* p);
    inline void callThreadStart() {LOCKIT(_guard); doCall(_threadStart);}
    inline void callThreadEnd() {LOCKIT(_guard); doCall(_threadEnd);}
    inline void callProcessEnd() {doCall(_processEnd);}

    void bye(CallBackInfo* p);
    static BOOL Main(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
    friend BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
public:
    static Dll* instance();

    void registerThreadStartCallBack(PdllCallBack func, void* data);
    void registerThreadEndCallBack(PdllCallBack func, void* data);
    void registerProcessEndCallBack(PdllCallBack func, void* data);

    int allocTls();
    bool freeTls(int key);
    void* getTls(int key);
    bool setTls(int key, void* value);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DLL_H__
// vim: set et ts=4 ai :

