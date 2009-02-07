#include <windows.h>
#include "dll.h"
#include "utils.h"
#include "hookfuncs.h"

/**
 * Utility class used by the Dll singleton to store linked list of 
 * callbacks to be called on thread creation/termination and process
 * exit events
 */
class CallBackInfo
{
private:
    CallBackInfo* _next;
    PdllCallBack _func;
    void* _data;
public:
    CallBackInfo(CallBackInfo* next, PdllCallBack func, void* data): 
        _next(next), _func(func), _data(data){}
    CallBackInfo* call() const
    {
        if (!this)
            return 0;
        CallBackInfo* res = _next;
        if (_func)
            _func(_data);

        return res;
    }
    inline CallBackInfo* next(){return _next;}
};

Dll::~Dll()
{
    bye(_threadStart);
    bye(_threadEnd);
    bye(_processEnd);
}

void Dll::bye(CallBackInfo* p)
{
    while (p)
    {
        CallBackInfo* n = p->next();
        delete p;
        p = n;
    }
}

/**
 * Returns the pointer to the singleton instance.
 */
Dll* Dll::instance()
{
    static Dll* dll = 0;
    if (!dll)
    {
        dll = new Dll();
        dll->registerProcessEndCallBack(endDll, dll);
    }

    return dll;
}

/**
 * The end process callback. Deletes the Dll instance.
 * @param[in] dll points to the singleton instance to delete
 */
void Dll::endDll(void* dll)
{
    if (dll)
        delete (Dll*)dll;
}

/**
 * Register a callback function to be called when new thread is started.
 *
 * @param[in] func points to the callback.
 * @param[in] userData points to user data to be passed to the callback.
 */
void Dll::registerThreadStartCallBack(PdllCallBack func, void* data)
{
    _threadStart = new CallBackInfo(_threadStart, func, data);
}

/**
 * Register a callback function to be called when the current thread is about to exit.
 *
 * @param[in] func points to the callback.
 * @param[in] userData points to user data to be passed to the callback.
 */
void Dll::registerThreadEndCallBack(PdllCallBack func, void* data)
{
    _threadEnd = new CallBackInfo(_threadEnd, func, data);
}

/**
 * Register a callback function to be called on process exit.
 *
 * @param[in] func points to the callback.
 * @param[in] userData points to user data to be passed to the callback.
 */
void Dll::registerProcessEndCallBack(PdllCallBack func, void* data)
{
    _processEnd = new CallBackInfo(_processEnd, func, data);
}

void Dll::doCall(CallBackInfo* p)
{
    for (;p; p = p->call());
}

BOOL Dll::Main(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        Dll::instance();
        InitDbgTrace();
        return TRUE;
    case DLL_PROCESS_DETACH:
        FreeDbgTrace();
        Dll::instance()->callProcessEnd();
        return TRUE;
    case DLL_THREAD_ATTACH:
        Dll::instance()->callThreadStart();
        return TRUE;
    case DLL_THREAD_DETACH:
        FreeDbgThreadBuff();
        Dll::instance()->callThreadEnd();
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return Dll::Main(hinstDLL, fdwReason, lpvReserved);
}

// vim: set et ts=4 ai :

