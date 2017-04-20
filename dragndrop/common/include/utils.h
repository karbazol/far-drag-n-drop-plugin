/**
 * @file utils.h
 * Contains declarations of misc utils.
 * 
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__UTILS_H__
#define __KARBAZOL_DRAGNDROP_2_0__UTILS_H__

/**
 * Return length of staticly defined array
 */
#define LENGTH(x) \
    (sizeof(x)/sizeof(*x))

void* getModuleBaseAddress(void* moduleItem);
/**
 * Returns base address of executable from which it is called.
 */
inline void* getMyModuleBaseAddress()
{
    return getModuleBaseAddress(&getMyModuleBaseAddress);
}
void* getEveryOneDescriptor();
void freeEveryOneDescriptor(void*);

#ifdef _DEBUG
void DbgAssert(const char* pCondition,const char* pFileName, int iLine);
#ifndef ASSERT

/**
 * Plug-in's defination of assertion macro.
 */
#define ASSERT(_x_) \
    if (!(_x_))         \
        DbgAssert(#_x_,__FILE__,__LINE__)
#endif

#else

#ifndef ASSERT
#define ASSERT __noop
#endif

#endif

#if defined(_DEBUG)||defined(ENABLE_TRACE)
void DbgTrace(const char*,...);
void InitDbgTrace();
void FreeDbgThreadBuff();
void FreeDbgTrace();
void DumpLastError(const char* fileName, int Line);
void DumpError(const char* fileName, int Line, unsigned int error);
void DumpClipboardFormat(unsigned int format);
void DumpIid(const char* fileName, int line, const char* function, const void* guid);

/**
 * Dumps a specified string to the debug output. See DbgTrace for parameters.
 */
#define TRACE DbgTrace
/**
 * Macro to dump a thread last error
 */
#define LASTERROR() \
    DumpLastError(__FILE__, __LINE__)

/**
 * Macro to dump an error                                                                
 */
#define DUMPERROR(x) \
    DumpError(__FILE__, __LINE__, x)

/**
 * Macro to dump a clipboard value to debug output
 */
#define TRACECBFMT \
    DumpClipboardFormat

/**
 * Macro to dump an interface id value to debug output
 */
#define TRACEIID(x) \
    DumpIid(__FILE__,__LINE__,__FUNCTION__,x)
#else
#define InitDbgTrace __noop
#define FreeDbgThreadBuff __noop
#define FreeDbgTrace __noop
#define TRACE __noop
#define LASTERROR __noop
#define DUMPERROR __noop
#define TRACECBFMT __noop
#define TRACEIID __noop
#endif

/**
 * Dumps "Not yet implemented" string to debug output
 */
#define NYI() \
    TRACE("[%s:%d:%s] Not yet implemented\n", __FILE__, __LINE__, __FUNCTION__)

/**
 * Dumps current function name to the debug output
 */
#define TRACE_FUNC() \
    TRACE("[%s:%d:%s] Entered function\n", __FILE__, __LINE__, __FUNCTION__)

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#endif // __KARBAZOL_DRAGNDROP_2_0__UTILS_H__
// vim: set et ts=4 ai :

