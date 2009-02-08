#ifndef __KARBAZOL_DRAGNDROP_2_0__FLDSPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__FLDSPRCS_H__

#include "fmtprcsr.h"

/**
 * Class to process data of CF_FILEDESCRIPTOR clipboard format.
 */
class FileDescriptorProcessor: public FormatProcessor
{
private:
    FileDescriptorProcessor(): FormatProcessor(){}
    HRESULT copyItem(IDataObject* obj, const wchar_t* name, FILEDESCRIPTOR* desc, int index);
    HRESULT handle(IStorage* stg, const wchar_t* name);
    HRESULT handle(IStream* stm, const wchar_t* name);
    HRESULT handleDir(const wchar_t* name, DWORD attr);
public:
    HRESULT operator()(IDataObject*, DWORD*);
    static inline FormatProcessor* create(){return new FileDescriptorProcessor();}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FLDSPRCS_H__

