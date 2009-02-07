#ifndef __KARBAZOL_DRAGNDROP_2_0__FLDSITRT_H__
#define __KARBAZOL_DRAGNDROP_2_0__FLDSITRT_H__

#include <windows.h>
#include <shlobj.h>
#include "mystring.h"

class FileDescriptorIterator
{
private:
    STGMEDIUM _medium;
    int _count;
    int _current;
    char* _pcurrent;
    int _step;
    MyStringW nameFromW();
    MyStringW nameFromA();
    typedef MyStringW (FileDescriptorIterator::*NameGetter)();
    NameGetter _name;
public:
    FileDescriptorIterator();
    ~FileDescriptorIterator();
    HRESULT initialize(IDataObject*);
    bool next();
    inline FILEDESCRIPTOR* value(){return (FILEDESCRIPTOR*)_pcurrent;}
    inline MyStringW name() {return (this->*_name)();}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FLDSITRT_H__

