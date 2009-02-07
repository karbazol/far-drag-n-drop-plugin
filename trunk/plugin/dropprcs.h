/**
 * @file dropprcs.h
 * Contains declaration of DropProcessor class.
 * 
 * $Id: dropprcs.h 64 2008-05-11 16:57:25Z eleskine $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__

#include <windows.h>
#include <shlobj.h>
#include "growarry.h"
#include "mystring.h"
#include "myshptr.h"

/**
 * Unicode strings array.
 */
typedef GrowOnlyArray<MyStringW> Strings;

class FormatProcessor;

/**
 * @brief Processes dropped data.
 *
 * Class is the main entry to process data dropped on Far.
 */
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
