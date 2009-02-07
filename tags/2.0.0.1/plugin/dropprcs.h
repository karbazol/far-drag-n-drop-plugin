#ifndef __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__

#include <windows.h>
#include <shlobj.h>
#include "growarry.h"
#include "mystring.h"
#include "myshptr.h"

typedef GrowOnlyArray<MyStringW> Strings;

class FormatProcessor;

class DropProcessor
{
private:
    DropProcessor(){}
    ~DropProcessor(){}
    static void kill(DropProcessor*);
    HRESULT handleAsync(IDataObject*, DWORD* effect, const wchar_t* destDir);
public:
    static DropProcessor* instance();
    HRESULT processDrop(IDataObject*, DWORD* effect, const wchar_t* destDir);
    HRESULT canProcess(IDataObject*);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__
