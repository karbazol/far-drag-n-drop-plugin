#include <common/shutils.h>
#include <common/utils.h>

#include "fmtprcsr.h"
#include "hdrpprcs.h"
#include "fldsprcs.h"

FormatProcessor* FormatProcessor::create(IDataObject* obj, const wchar_t* destDir)
{
    struct
    {
        CLIPFORMAT fmt;
        FormatCreator creator;
    } fmts[] =
    {
        {CF_HDROP, &HdropProcessor::create},
        {CF_FILEDESCRIPTORW, &FileDescriptorProcessor::create},
        {CF_FILEDESCRIPTORA, &FileDescriptorProcessor::create},
    };

    size_t i;
    for (i = 0; i < LENGTH(fmts); i++)
    {
        FORMATETC fmt = {fmts[i].fmt, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM data = {0};
        if (SUCCEEDED(obj->GetData(&fmt, &data)))
        {
            ReleaseStgMedium(&data);
            FormatProcessor* res = fmts[i].creator();
            res->_dir = destDir;
            return res;
        }
    }

    return 0;
}

// vim: set et ts=4 ai :

