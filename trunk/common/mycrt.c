/**
 * @file mycrt.c
 * The file contains a subset of crt routines used by the plugin.
 *
 * $Id: mycrt.c 64 2008-05-11 16:57:25Z eleskine $
 */

#include <stdlib.h>
#include <windows.h>

#ifndef _DEBUG
#ifndef DOXYGEN_SHOULD_SKIP_THIS

__declspec(noalias) __declspec(restrict) void* malloc(size_t size)
{
//    return HeapAlloc(GetProcessHeap(), 0, size);
    return CoTaskMemAlloc(size);
}

__declspec(noalias) void free(void* p)
{
//    HeapFree(GetProcessHeap(), 0, p);
    CoTaskMemFree(p);
}

__declspec(noalias) __declspec(restrict) void* realloc(void* p, size_t size)
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
