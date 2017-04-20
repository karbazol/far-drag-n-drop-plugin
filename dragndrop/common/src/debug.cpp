/**
 * @file debug.cpp
 * The file contains routines for debug purposes.
 */

#include <windows.h>
#include <shlwapi.h>
#include <utils.h>

#ifdef _DEBUG
/**
 * Assert macro support function
 */
void DbgAssert(const char* pCondition,const char* pFileName, int iLine)
{
    char szInfo[4096]; // 4096 is the max buffer wsprintf can accept
    wsprintfA(szInfo, "%s \nAt line %d of %s\nContinue? (Cancel to debug)",
        pCondition, iLine, pFileName);
    INT MsgId = MessageBoxA(NULL,szInfo,"ASSERT Failed",
        MB_SYSTEMMODAL |
        MB_ICONHAND |
        MB_YESNOCANCEL |
        MB_SETFOREGROUND);
    switch (MsgId)
    {
    case IDNO:              /* Kill the application */

        FatalAppExitA(FALSE, "Application terminated");
        break;

    case IDCANCEL:          /* Break into the debugger */
        {
            __try
            {
                DebugBreak();
            }
            __except(UnhandledExceptionFilter(GetExceptionInformation()))
            {
            }
        }
        break;

    case IDYES:             /* Ignore assertion continue execution */
        break;
    }
}
#endif


#if defined(_DEBUG)||defined(ENABLE_TRACE)
static char debugProcessName[4096]={0};
static char* processName=0;
static char debugModuleName[4096]={0};
static char* moduleName=0;

/**
 * @brief Debug string buffers
 *
 * Utility structure used by DbgTrace function to store per-thread buffers.
 */
struct DebugBuffs
{
    char buff0[4096];
    char buff1[4096];
    char* pBuff;
    DebugBuffs()
    {
        memset(buff0, 0, sizeof(buff0));
        memset(buff1, 0, sizeof(buff1));
        pBuff = buff0;
    }
    void* operator new(size_t size)
    {
        return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    }
    void operator delete(void* p)
    {
        if (p)
        {
            VirtualFree(p, 0, MEM_RELEASE);
        }
    }
};

static bool getModuleName(void* module, char* modulePath, size_t modulePathSize, char*& moduleFileName)
{
    if (GetModuleFileNameA(reinterpret_cast<HMODULE>(module), modulePath,
                static_cast<DWORD>(modulePathSize)))
    {
        moduleFileName = StrRChrA(modulePath, NULL, '\\');
        if (moduleFileName)
            moduleFileName++;
        return true;
    }
    else
    {
        return false;
    }
}

static DWORD tlsBuffIdx = TLS_OUT_OF_INDEXES;

/**
 * Initializes DbgTrace API
 */
void InitDbgTrace()
{
    tlsBuffIdx = TlsAlloc();

    if (!getModuleName(NULL, debugProcessName, LENGTH(debugProcessName), processName))
        processName = "unknown";
    if (!getModuleName(getMyModuleBaseAddress(), debugModuleName, LENGTH(debugModuleName), moduleName))
        moduleName = "unknown";
}

static DebugBuffs* GetDbgThreadBuff()
{
    DebugBuffs* res = reinterpret_cast<DebugBuffs*>(TlsGetValue(tlsBuffIdx));
    if (!res)
    {
        res = new DebugBuffs();
        if (res)
        {
            TlsSetValue(tlsBuffIdx, res);
        }
    }
    return res;
}

/**
 * This function should be called just before the module it uses is about
 * to dettach from one thread.
 */
void FreeDbgThreadBuff()
{
    DebugBuffs* p = reinterpret_cast<DebugBuffs*>(TlsGetValue(tlsBuffIdx));
    delete p;
    TlsSetValue(tlsBuffIdx, 0);
}

/**
 * This function should be called just before the module it uses is about
 * to unload.
 */
void FreeDbgTrace()
{
    FreeDbgThreadBuff();
    TlsFree(tlsBuffIdx);
}

/**
 * Function outputs debug information using standard OutputDebugString
 * @param[in] lpszFormat Format specifying string to generate a string
 * using elipsis arguments
 */
void DbgTrace(const char* lpszFormat,...)
{
    va_list va;
    va_start(va, lpszFormat);

    DebugBuffs* p = GetDbgThreadBuff();

    if (!p)
    {
        /** Something is wrong */
        DebugBreak();
        return;
    }

    int written = wvnsprintfA(p->pBuff, static_cast<int>(sizeof(p->buff0) - (p->pBuff - p->buff0))-1, lpszFormat, va);
    if (written > 0)
    {
        p->pBuff += written;

        char* pN = StrChrA(p->buff0, '\n');

        while (pN != NULL || (sizeof(p->buff0) - (p->pBuff - p->buff0)) < 64)
        {
            char oldN = *++pN;
            *pN = '\0';
            if (!processName)
            {
                OutputDebugStringA(p->buff0);
            }
            else
            {
                wnsprintfA(p->buff1, sizeof(p->buff1)-1, "[%s:%s:%d]%s", processName, moduleName, GetCurrentThreadId(), p->buff0);
                OutputDebugStringA(p->buff1);
            }
            *pN = oldN;
            lstrcpynA(p->buff1, pN, LENGTH(p->buff1));
            lstrcpynA(p->buff0, p->buff1, LENGTH(p->buff0));
            p->pBuff = p->buff0 + lstrlenA(p->buff0);
            pN = StrChrA(p->buff0, '\n');
        }
    }
}

/**
 * Dumps a specified error to debug output
 * @param[in] fileName Path to source file where the error occured
 * @param[in] line File line number where the error occured
 * @param[in] error The error number
 */
void DumpError(const char* fileName, int line, unsigned int error)
{
    char* msg=NULL;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ARGUMENT_ARRAY,
            NULL, error, 0, (LPSTR)&msg, 0, NULL);

    if (msg)
    {
        size_t i = lstrlenA(msg) - 1;
        if (msg[i] == '\n')
            msg[i] = 0;
    }

    DbgTrace("[%s:%d] System error: 0x%.8x %s\n",
            fileName, line, error, msg?msg:"");

    if (msg)
        LocalFree(msg);

}

/**
 * Dumps the last thread error
 * @param[in] fileName Path to source file where the error occured
 * @param[in] line File line number where the error occured
 */
void DumpLastError(const char* fileName, int line)
{
    DWORD error = GetLastError();

    DumpError(fileName, line, error);

    SetLastError(error);
}

/**
 * Dumps the clipboard format name to the debug output
 * @param[in] format Clipboard format number
 */
void DumpClipboardFormat(unsigned int format)
{
    static struct {unsigned int _format; const char* _name;}
    formatNames [] =
    {
        { CF_TEXT,            "CF_TEXT" },
        { CF_BITMAP,          "CF_BITMAP" },
        { CF_METAFILEPICT,    "CF_METAFILEPICT" },
        { CF_SYLK,            "CF_SYLK" },
        { CF_DIF,             "CF_DIF" },
        { CF_TIFF,            "CF_TIFF" },
        { CF_OEMTEXT,         "CF_OEMTEXT" },
        { CF_DIB,             "CF_DIB" },
        { CF_PALETTE,         "CF_PALETTE" },
        { CF_PENDATA,         "CF_PENDATA" },
        { CF_RIFF,            "CF_RIFF" },
        { CF_WAVE,            "CF_WAVE" },
        { CF_UNICODETEXT,     "CF_UNICODETEXT" },
        { CF_ENHMETAFILE,     "CF_ENHMETAFILE" },
        { CF_HDROP,           "CF_HDROP" },
        { CF_LOCALE,          "CF_LOCALE" },
        { CF_MAX,             "CF_MAX" },
        { CF_OWNERDISPLAY,    "CF_OWNERDISPLAY" },
        { CF_DSPTEXT,         "CF_DSPTEXT" },
        { CF_DSPBITMAP,       "CF_DSPBITMAP" },
        { CF_DSPMETAFILEPICT, "CF_DSPMETAFILEPICT" },
        { CF_DSPENHMETAFILE,  "CF_DSPENHMETAFILE" },
    };
    char name[MAX_PATH]={0};

    if (!GetClipboardFormatNameA(format, name, LENGTH(name)))
    {
        wnsprintfA(name, LENGTH(name), "0x%.8x", format);
        size_t i;
        for (i = 0; i < LENGTH(formatNames); i++)
        {
            if (formatNames[i]._format == format)
            {
                lstrcpyA(name, formatNames[i]._name);
                break;
            }
        }
    }

    DbgTrace("%s", name);
}

/**
 * Dumps IID value to the debug output
 */
void DumpIid(const char* fileName, int line, const char* function, const void* guid)
{
    WCHAR* str;
    if (FAILED(StringFromIID(*reinterpret_cast<const IID*>(guid), &str)))
    {
        str = NULL;
    }

    DbgTrace("[%s:%d:%s] GUID: %S\n", fileName, line, function, str?str:L"Unknown");

    if (str)
    {
        CoTaskMemFree(str);
    }
}

/**
 * Dumps Window information to the debug output
 */
void DumpWindow(const char* /*fileName*/, int /*line*/, const char* /*function*/, const void* /*wnd*/)
{

}

#endif
