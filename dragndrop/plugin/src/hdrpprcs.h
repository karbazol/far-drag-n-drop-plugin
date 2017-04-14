#ifndef __KARBAZOL_DRAGNDROP_2_0__HDRPPRCS_H__
#define __KARBAZOL_DRAGNDROP_2_0__HDRPPRCS_H__

#include "fmtprcsr.h"
#include "filelist.h"
#include "cpydlg.h"

/**
 * Class to process data of CF_HDROP clipboard format.
 */
class HdropProcessor: public FormatProcessor
{
private:
    HdropProcessor(): FormatProcessor(){}
    HRESULT shellCopyOrMoveHDrop(MyStringW& files, bool move);
    HRESULT farCopyOrMoveHDropRecursively(MyStringW& files, bool move);
    HRESULT farMoveHDrop(MyStringW& files);
    HRESULT shellLinkHDrop(MyStringW& files);
    bool initStringInfo(MyStringW& s, HGLOBAL hDrop);
    HRESULT processDir(const FileListEntry& e);
    HRESULT processFile(const FileListEntry& e, CopyDialog* dialog, bool move);
public:
    HRESULT operator()(IDataObject*, DWORD*);
    static inline FormatProcessor* create(){return new HdropProcessor();}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__HDRPPRCS_H__

