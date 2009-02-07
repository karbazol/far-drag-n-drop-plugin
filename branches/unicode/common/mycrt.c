/**
 * @file mycrt.c
 * The file contains a subset of crt routines used by the plugin.
 *
 * $Id: mycrt.c 88 2009-01-09 15:20:59Z eleskine $
 */

#include <stdlib.h>
#include <windows.h>

#ifndef _DEBUG
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#if _MSC_VER >= 1400
__declspec(noalias) __declspec(restrict)
#endif
void* malloc(size_t size)
{
//    return HeapAlloc(GetProcessHeap(), 0, size);
    return CoTaskMemAlloc(size);
}

#if _MSC_VER >= 1400
__declspec(noalias) 
#endif
void free(void* p)
{
//    HeapFree(GetProcessHeap(), 0, p);
    CoTaskMemFree(p);
}

#if _MSC_VER >= 1400
__declspec(noalias) __declspec(restrict)
#endif
void* realloc(void* p, size_t size)
{
//    return HeapReAlloc(GetProcessHeap(), 0, p, size);
    return CoTaskMemRealloc(p, size);
}

void __cdecl _purecall(
        void
        )
{
    FatalAppExit(0, L"Pure Function Call");
}

#ifdef RtlMoveMemory
#undef RtlMoveMemory
#endif

void WINAPI RtlMoveMemory ( void *, const void *, size_t count );

#ifdef memmove
#undef memmove
#endif

void * __cdecl memmove (
        void * dst,
        const void * src,
        size_t count
        )
{
    void * ret = dst;

    {
        RtlMoveMemory(dst, src, count);
    }

    return ret;
}

#endif // DOXYGEN_SHOULD_SKIP_THIS
#endif // _DEBUG

// vim: set et ts=4 ai :
