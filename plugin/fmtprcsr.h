#ifndef __KARBAZOL_DRAGNDROP_2_0__FMTPRCSR_H__
#define __KARBAZOL_DRAGNDROP_2_0__FMTPRCSR_H__

#include <windows.h>
#include <shlobj.h>
#include "dircrtr.h"

/**
 * Singleton. Class-factory. Represents a manager of different clipboard format
 * processors.
 */
class FormatProcessor
{
private:
    DirectoryCreator _dir;
protected:
    FormatProcessor(): _dir(){}
private:
    typedef FormatProcessor* (*FormatCreator)();
public:
    virtual ~FormatProcessor(){}

    /**
     * Processes a data object.
     */
    virtual HRESULT operator()(IDataObject* obj, DWORD* effect) = 0;

    /**
     * Returns reference to internal DirectoryCreator
     */
    inline const DirectoryCreator& dir() const {return _dir;}

    /**
     * Returns pointer to newly created format processor.
     */
    static FormatProcessor* create(IDataObject* obj, const wchar_t* destDir);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FMTPRCSR_H__

