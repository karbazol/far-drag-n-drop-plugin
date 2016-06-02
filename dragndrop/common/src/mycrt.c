/**
 * @file mycrt.c
 * The file contains a subset of crt routines used by the plugin.
 *
 * $Id: mycrt.c 77 2011-10-22 20:25:46Z Karbazol $
 */

#include <stdlib.h>
#include <windows.h>

#ifndef _DEBUG
#ifndef DOXYGEN_SHOULD_SKIP_THIS

static HANDLE getMyHeap()
{
    ULONG  HeapFragValue = 2;
    static HANDLE heap = 0;
    if (!heap)
    {
        heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 1024*1024, 0);
        HeapSetInformation(heap,
                       HeapCompatibilityInformation,
                       &HeapFragValue,
                       sizeof(HeapFragValue));
    }

    return heap;
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
        return HeapReAlloc(getMyHeap(), HEAP_GENERATE_EXCEPTIONS, p, size);
    }
    else
    {
        return HeapAlloc(getMyHeap(), HEAP_GENERATE_EXCEPTIONS, size);
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


#ifdef _M_IX86_ // These two functions should be picked up from libcmt
LONGLONG _alldiv(LONGLONG a, LONGLONG b)
{
    return 0;
}

LONGLONG _allmul(LONGLONG a, LONGLONG b)
{
    return 0;
}
#endif

#endif // DOXYGEN_SHOULD_SKIP_THIS
#endif // _DEBUG

// vim: set et ts=4 ai :
