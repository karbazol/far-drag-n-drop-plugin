/**
 * @file dropprcs.h
 * Contains declaration of DropProcessor class.
 * 
 * $Id: dropprcs.h 77 2011-10-22 20:25:46Z Karbazol $
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__DROPPRCS_H__

#include <windows.h>
#include <shlobj.h>
#include <common/growarry.h>
#include <common/myshptr.h>
#include <dll/mystring.h>

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
