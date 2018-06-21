/**
 * @file mycrt.c
 * The file contains a subset of crt routines used by the plugin.
 *
 */

#include <stdlib.h>
#include <windows.h>

#ifndef _DEBUG
#ifndef DOXYGEN_SHOULD_SKIP_THIS

static HANDLE getMyHeap()
{
    return GetProcessHeap();
}

#if _MSC_VER >= 1400
__declspec(noalias) __declspec(restrict)
#endif
void* malloc(size_t size)
{
    return HeapAlloc(getMyHeap(), 0, size);
//    return CoTaskMemAlloc(size);
}

#if _MSC_VER >= 1400
__declspec(noalias)
#endif
void free(void* p)
{
    if (p)
    {
        HeapFree(getMyHeap(), 0, p);
//        CoTaskMemFree(p);
    }
}

#if _MSC_VER >= 1400
__declspec(noalias) __declspec(restrict)
#endif
void* realloc(void* p, size_t size)
{
    if (p)
    {
        return HeapReAlloc(getMyHeap(), 0, p, size);
    }
    else
    {
        return HeapAlloc(getMyHeap(), 0, size);
    }
//    return CoTaskMemRealloc(p, size);
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

#pragma function(memmove)
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

#ifdef memcpy
#undef memcpy
#endif

#pragma function(memcpy)
void * __cdecl memcpy (
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

#ifdef RtlFillMemory
#undef RtlFillMemory
#endif

void WINAPI RtlFillMemory ( void *, size_t, unsigned char );

#ifdef memset
#undef memset
#endif

#pragma function(memset)
void * __cdecl memset (
        void * dst,
        int ch,
        size_t count
        )
{
    RtlFillMemory(dst, count, ch);
    return dst;
}

#endif // DOXYGEN_SHOULD_SKIP_THIS
#endif // _DEBUG

// vim: set et ts=4 ai :
