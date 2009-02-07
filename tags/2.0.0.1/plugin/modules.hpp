#ifndef __KARBAZOL_DRAGNDROP_2_0__MODULES_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__MODULES_HPP__

class ModuleWalker
{
public:
    virtual ~ModuleWalker(){}
    // Interface
    virtual void reset() = 0;
    virtual bool next() = 0;
    virtual void* func() const = 0;
    virtual bool func(void* value) = 0;
    virtual bool isName(const char* value) const = 0;
};

ModuleWalker* ImportsWalker(void* module);

class AutoWalker
{
private:
    ModuleWalker* _p;
public:
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

    // disable copy constructor
    AutoWalker(const AutoWalker&);
    // disable assignment
    AutoWalker& operator =(const AutoWalker&);

    inline ModuleWalker* operator->(){return _p;};
};

#endif // __KARBAZOL_DRAGNDROP_2_0__MODULES_HPP__

// vim: set et ts=4 ai :

