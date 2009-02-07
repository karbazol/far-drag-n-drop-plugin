#include "modules.hpp"
#include "protect.h"

class CommonWalker
{
protected:
    void* _module;
    MemProtect* _protect;
public:
    CommonWalker(void* module): _module(module), _protect(0){}
    ~CommonWalker()
    {
        if (_protect)
        {
            delete _protect;
        }
    }
};

class ModuleImportsWalker : public CommonWalker, public ModuleWalker
{
protected:
    void** _current;
    int* _names;
public:
    ModuleImportsWalker(void* module): CommonWalker(module), _current(0), _names(0){}
    ~ModuleImportsWalker(){}
    void reset()
    {
        if (!_module)
            return;
        PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)_module;
        PIMAGE_NT_HEADERS pNT = (PIMAGE_NT_HEADERS)((DWORD_PTR)pDos + pDos->e_lfanew);
        PIMAGE_IMPORT_DESCRIPTOR pImports = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)pDos +
            pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        _current = (void**)((DWORD_PTR)pDos + pImports->FirstThunk);
        _protect = new MemProtect(_current, PAGE_EXECUTE_READWRITE);
        if (pImports->OriginalFirstThunk)
            _names = (int*)((DWORD_PTR)pDos + pImports->OriginalFirstThunk);
    }
    bool next()
    {
        if (!_current)
            return false;

        if (!*_current++)
            return false;

        if (_names)
            _names++;

        return true;
    }
    void* func() const
    {
        if (!_current)
            return 0;
        return *_current;
    }
    bool func(void* value)
    {
        if (!value || !_current || !*_current)
            return false;
        *_current = value;
        return true;
    }
    bool isName(const char* value) const
    {
        if (!value || !_names || !*_names)
            return false;

        return lstrcmpA((char*)_module + *_names + 2, value)==0;
    }
};

ModuleWalker* ImportsWalker(void* module)
{
    return new ModuleImportsWalker(module);
}

// vim: set et ts=4 ai :

