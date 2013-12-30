/**
 * @file modules.hpp
 * Contains declarations of public classes and functions used to
 * manipulate with modules' imports/exports sections.
 *
 * $Id: modules.hpp 77 2011-10-22 20:25:46Z Karbazol $
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__MODULES_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__MODULES_HPP__

/**
 * Abstract class which allows to walk through module's imports/exports section.
 */
class ModuleWalker
{
public:
    virtual ~ModuleWalker(){}
    
    /**
     * Allows to begin walking from the beginning.
     */
    virtual void reset() = 0;

    /**
     * Goes to the next entry in the section being walked through.
     * @return true if current is valid entry and false if walker passed end of section.
     */
    virtual bool next() = 0;

    /**
     * Current function getter.
     * @return VA if the walker is on valid entry otherwise NULL.
     */
    virtual void* func() const = 0;

    /**
     * Current function setter.
     * @param[in] new VA of function under walker.
     * @return true if walker points to valid entry.
     */
    virtual bool func(void* value) = 0;

    /**
     * Checks wheather the specified name is under cursor.
     * @param[in] value String representing a name to check.
     * @return true if the function name under cursor is the same as value.
     */
    virtual bool isName(const char* value) const = 0;

    virtual const char* funcName() const = 0;
    virtual const char* modName() const = 0;
};

/**
 * Function to get a walker on specified module.
 * 
 * @param[in] module Specifies base module which imports section should be walked.
 * @return pointer to ModuleWalker class instance.
 */
ModuleWalker* ImportsWalker(void* module);

/**
 * Wrapper class used to manage instances of ModuleWalker class
 */
class AutoWalker
{
private:
    ModuleWalker* _p;
public:
    /**
     * @brief Constructor
     *
     * @param[in] p Pointer to ModuleWalker instance to use
     */
    AutoWalker(ModuleWalker* p): _p(p)
    {
        if (_p)
            _p->reset();
    }
    ~AutoWalker()
    {
        if (_p)
            delete _p;
    }

    /**
     * @brief Disabled copy constructor
     */
    AutoWalker(const AutoWalker&);
    
    /**
     * @brief Disable assignment operator
     */
    AutoWalker& operator =(const AutoWalker&);

    /**
     * @return stored pointer of type ModuleWalker*
     */
    inline ModuleWalker* operator->(){return _p;};
};

/**
 * The structure represents a single patch entry.
 */
struct PatchInfo
{
    void* func;
    void* patch;
    const char* name;
};

bool patchModuleImports(void* module, PatchInfo* patches, size_t count);
bool restoreModuleImports(void* module, PatchInfo* patches, size_t count);

#endif // __KARBAZOL_DRAGNDROP_2_0__MODULES_HPP__

// vim: set et ts=4 ai :

