#ifndef __KARBAZOL_DRAGNDROP_2_0__UTILS_H__
#define __KARBAZOL_DRAGNDROP_2_0__UTILS_H__

#define LENGTH(x) (sizeof(x)/sizeof(*x))

void* getMyModuleBaseAddress();
void* getEveryOneDescriptor();
void freeEveryOneDescriptor(void*);

#ifdef _DEBUG
void DbgTrace(const char*,...);
void DbgAssert(const char* pCondition,const char* pFileName, int iLine);
void InitDbgTrace();
void FreeDbgThreadBuff();
void FreeDbgTrace();
void DumpLastError(const char* fileName, int Line);
void DumpError(const char* fileName, int Line, unsigned int error);
void DumpClipboardFormat(unsigned int format);
void DumpIid(const char* fileName, int line, const char* function, const void* guid);
#define TRACE DbgTrace
#ifndef ASSERT
#define ASSERT(_x_) if (!(_x_))         \
        DbgAssert(#_x_,__FILE__,__LINE__)
#endif
#define LASTERROR() DumpLastError(__FILE__, __LINE__)
#define DUMPERROR(x) DumpError(__FILE__, __LINE__, x)
#define TRACECBFMT DumpClipboardFormat
#define TRACEIID(x) DumpIid(__FILE__,__LINE__,__FUNCTION__,x)
#else
#define InitDbgTrace __noop
#define FreeDbgThreadBuff __noop
#define FreeDbgTrace __noop
#define TRACE __noop
#ifndef ASSERT
#define ASSERT __noop
#endif
#define LASTERROR __noop
#define DUMPERROR __noop
#define TRACECBFMT __noop
#define TRACEIID __noop
#endif

#define NYI() TRACE("[%s:%d:%s] Not yet implemented\n", __FILE__, __LINE__, __FUNCTION__)
#define TRACE_FUNC() TRACE("[%s:%d:%s] Entered function\n", __FILE__, __LINE__, __FUNCTION__)

void normalizePath(wchar_t* path);

#endif // __KARBAZOL_DRAGNDROP_2_0__UTILS_H__
// vim: set et ts=4 ai :

