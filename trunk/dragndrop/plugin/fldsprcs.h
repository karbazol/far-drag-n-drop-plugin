#ifndef __KARBAZOL_DRAGNDROP_2_0__FLDSPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__FLDSPRCS_H__

#include "fmtprcsr.h"

class FileDescriptorIterator;

/**
 * Class to process data of CF_FILEDESCRIPTOR clipboard format.
 */
class FileDescriptorProcessor: public FormatProcessor
{
private:
    FileDescriptorProcessor(): FormatProcessor(){}
    HRESULT copyItem(IDataObject* obj, FileDescriptorIterator& file, int index);
    HRESULT updateItemTimes(FileDescriptorIterator& file);
    HRESULT handle(IStorage* stg, FileDescriptorIterator& file);
    HRESULT handle(IStream* stm, FileDescriptorIterator& file);
    HRESULT handleDir(FileDescriptorIterator& file);
public:
    HRESULT operator()(IDataObject*, DWORD*);
    static inline FormatProcessor* create(){return new FileDescriptorProcessor();}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FLDSPRCS_H__

