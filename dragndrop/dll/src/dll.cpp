/**
 * @file dll.cpp
 * Contains implementation of Dll class
 *
 * $Id: dll.cpp 77 2011-10-22 20:25:46Z Karbazol $
 */

#include <windows.h>

#include <common/utils.h>
#include <dll/dll.h>

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
    /**
     * Constructor
     * @param[in] next Points to the next CallBackInfo object in the list
     * @param[in] func Pointer to user function to be called by call() method.
     * @param[in] data Pointer to be passed to the user functions
     */
    CallBackInfo(CallBackInfo* next, PdllCallBack func, void* data): 
        _next(next), _func(func), _data(data){}

    /**
     * Calls user function
     */
    CallBackInfo* call() const
    {
        if (!this)
            return 0;
        CallBackInfo* res = _next;
        if (_func)
            _func(_data);

        return res;
    }

    /**
     * @return pointer to the next CallBackInfo object in the list
     */
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
        if (dll)
        {
            dll->registerProcessEndCallBack(endDll, dll);
        }
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
    {
        delete reinterpret_cast<Dll*>(dll);
    }
}

/**
 * Register a callback function to be called when new thread is started.
 *
 * @param[in] func points to the callback.
 * @param[in] userData points to user data to be passed to the callback.
 */
void Dll::registerThreadStartCallBack(PdllCallBack func, void* data)
{
    LOCKIT(_guard);
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
    LOCKIT(_guard);
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
    LOCKIT(_guard);
    _processEnd = new CallBackInfo(_processEnd, func, data);
}

void Dll::doCall(CallBackInfo* p)
{
    for (;p; p = p->call());
}

#if _MSC_VER >= 1400  && defined(NDEBUG) && defined(_M_X86)
extern "C" int __sse2_available_init();
#endif

BOOL Dll::Main(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
    Dll* dll = Dll::instance();
    if (!dll)
    {
        return FALSE;
    }

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
#if _MSC_VER >= 1400  && defined(NDEBUG)
#if defined(_M_X86)
        __sse2_available_init();
#endif
#endif

        InitDbgTrace();
        return TRUE;
    case DLL_PROCESS_DETACH:
        FreeDbgTrace();
        dll->callProcessEnd();
        return TRUE;
    case DLL_THREAD_ATTACH:
        dll->callThreadStart();
        return TRUE;
    case DLL_THREAD_DETACH:
        FreeDbgThreadBuff();
        dll->callThreadEnd();
        return TRUE;
    default:
        return FALSE;
    }
}

/**
 * @brief DLL entry point.
 *
 * The DllMain function is an optional entry point into a dynamic-link library
 * (DLL). If the function is used, it is called by the system when processes 
 * and threads are initialized and terminated, or upon calls to the 
 * LoadLibrary and FreeLibrary functions.
 *
 * @param[in] hinstDLL Handle to the DLL module. The value is the base 
 * address of the DLL. The HINSTANCE of a DLL is the same as the HMODULE of 
 * the DLL, so hinstDLL can be used in calls to functions that require a module handle.
 *
 * @param[in] fdwReason Indicates why the DLL entry-point function is being called.
 *
 * @param[in] lpvReserved Reserverd by M$
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return Dll::Main(hinstDLL, fdwReason, lpvReserved);
}

// vim: set et ts=4 ai :

