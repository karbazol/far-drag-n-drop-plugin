#include "protect.h"

MemProtect::MemProtect(void* p, DWORD protect)
{
    if (VirtualQuery(p, &_mi, sizeof(_mi)))
    {
        
        if (!VirtualProtect(_mi.BaseAddress, _mi.RegionSize, protect, &_oldProtect))
            _mi.BaseAddress = 0;
    }
    else 
        _mi.BaseAddress = 0;
}

MemProtect::~MemProtect()
{
    if (_mi.BaseAddress)
        VirtualProtect(_mi.BaseAddress, _mi.RegionSize, _oldProtect, &_oldProtect);
}

// vim: set et ts=4 :

