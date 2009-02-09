// $Id$
#ifndef __KARBAZOL_DRAGNDROP_2_0__HDRPPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__HDRPPRCS_H__

#include "fmtprcsr.h"
#include "filelist.h"
#include "dlgnotfy.h"

/**
 * Class to process data of CF_HDROP clipboard format.
 */
class HdropProcessor: public FormatProcessor
{
private:
    HdropProcessor(): FormatProcessor(){}
    HRESULT shellCopyHDrop(MyStringW& files);
    HRESULT farCopyHDrop(MyStringW& files);
    bool initStringInfo(MyStringW& s, HGLOBAL hDrop);
    HRESULT processDir(const FileListEntry& e);
    HRESULT processFile(const FileListEntry& e, CopyDialogNotify* notify);
public:
    HRESULT operator()(IDataObject*, DWORD*);
    static inline FormatProcessor* create(){return new HdropProcessor();}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HDRPPRCS_H__

