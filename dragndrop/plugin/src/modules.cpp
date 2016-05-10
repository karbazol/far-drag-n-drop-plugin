/**
 * @file modules.cpp
 * Contains implementation of modules walker classes.
 *
 * $Id: modules.cpp 77 2011-10-22 20:25:46Z Karbazol $
 */

#include <common/utils.h>

#include "modules.hpp"
#include "protect.h"

/**
 * @brief MemProtect wrappers
 *
 * Used to manage MemProtect instance during manipulation on 
 * module sections.
 */
class CommonWalker
{
protected:
    /**
     * Base address of the module to walk through
     */
    void* _module;
    /**
     * Module protecter
     */
    MemProtect* _protect;
public:
    /**
     * @param[in] module Base address of the module to walk through
     */
    CommonWalker(void* module): _module(module), _protect(0){}
    ~CommonWalker()
    {
        if (_protect)
        {
            delete _protect;
        }
    }
};

/**
 * @brief Used to walk imports of specified module.
 *
 * Class implements iterator on specified module's imports section.
 * Used to modify imports entries.
 */
class ModuleImportsWalker : public CommonWalker, public ModuleWalker
{
protected:
    void** _current;
    int* _names;
    PIMAGE_IMPORT_DESCRIPTOR _importedModule;
public:
    ModuleImportsWalker(void* module): CommonWalker(module), _current(0), _names(0){}
    ~ModuleImportsWalker(){}
    void reset()
    {
        if (!_module)
            return;
        PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)_module;
        PIMAGE_NT_HEADERS pNT = (PIMAGE_NT_HEADERS)((DWORD_PTR)pDos + pDos->e_lfanew);
        _importedModule = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)pDos +
            pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        _current = (void**)((DWORD_PTR)pDos + _importedModule->FirstThunk);
        _protect = new MemProtect(_current, PAGE_EXECUTE_READWRITE);
        if (_importedModule->OriginalFirstThunk)
            _names = (int*)((DWORD_PTR)pDos + _importedModule->OriginalFirstThunk);
    }
    bool next()
    {
        if (!_current)
            return false;

        if (!*_current++)
        {
            _importedModule++;
            if (!_importedModule->OriginalFirstThunk)
                return false;
            _current = (void**)((DWORD_PTR)_module + _importedModule->FirstThunk);
            _names = (int*)((DWORD_PTR)_module + _importedModule->OriginalFirstThunk);
            return true;
        }

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

        return lstrcmpA(funcName(), value)==0;
    }
    const char* funcName() const
    {
        if (!_module || !_names || !*_names || (*_names & IMAGE_ORDINAL_FLAG))
            return "";
        char* res = (char*)_module + *_names + 2;
        if (!IsBadReadPtr(res, 1))
            return res;
        return "";
    }
    const char* modName() const
    {
        if (!_module || !_importedModule || !_importedModule->OriginalFirstThunk)
            return 0;
        return (char*)_module + _importedModule->Name;
    }
};

ModuleWalker* ImportsWalker(void* module)
{
    return new ModuleImportsWalker(module);
}

/**
 * @brief Patches specified module's imports section.
 *
 * The function walks all imports sections of the specified module
 * and replaces entries of specified function with user functions
 */
bool patchModuleImports(void* module, PatchInfo* patches, size_t count)
{
    AutoWalker m(ImportsWalker(module));
    do
    {
#ifdef _DEBUG__
        const char* modName = m->modName();
        const char* funcName = m->funcName();
        TRACE("Imported function %s::%s\n", modName, funcName);
#endif
        size_t i;
        for (i = 0; i < count; i++)
        {
            if (m->func() == patches[i].func || m->isName(patches[i].name))
            {
                patches[i].func = m->func();
                m->func(patches[i].patch);
                break;
            }
        }
    } while (m->next());
    
    return true; 
}

/**
 * @brief Restores specified module's imports section.
 *
 * The function walks all imports sections of the specified module
 * and replaces entries of specified function with original functions
 */
bool restoreModuleImports(void* module, PatchInfo* patches, size_t count)
{
    AutoWalker m(ImportsWalker(module));

    do
    {
        size_t i;
        for (i = 0; i < count; i++)
        {
            if (m->func() == patches[i].patch)
            {
                m->func(patches[i].func);
                break;
            }
        }
    } while (m->next());

    return true;
}

// vim: set et ts=4 ai :

