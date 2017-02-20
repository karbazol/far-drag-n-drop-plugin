#include <stdlib.h>

void* operator new(size_t size)
{
    return malloc(size);
}

void operator delete(void* p)
{
    free(p);
}

// nobody expects std::bad_alloc
void* operator new[](size_t size)
{
    return malloc(size);
}

void operator delete[](void* p)
{
    free(p);
}

void operator delete(void* p, size_t)
{
    free(p);
}

void operator delete[](void* p, size_t)
{
    free(p);
}

// vim: set et ts=4 ai :

