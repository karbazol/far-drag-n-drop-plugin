/**
 * @file datacont.h
 * Contains declaration of DataContainer class.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__DATACONT_H__
#define __KARBAZOL_DRAGNDROP_2_0__DATACONT_H__

#include "far.h"
#include "cstmdata.h"
#include "mystring.h"

/**
 * @brief Data container object
 *
 * Represents data container object used by DataObject to hold
 * file data during dragging operation
 */
class DataContainer
{
private:
    MyStringW _dir;
    MyStringW* _files;
    size_t _count;
    CustomData* _custom;
    size_t _customCount;
    size_t _customCapacity;
    void growCustom();
public:
    DataContainer(): _dir(), _files(NULL), _count(0), _custom(0),
        _customCount(0), _customCapacity(0){}
    DataContainer(const wchar_t* dir, const PluginPanelItemsW& /*items*/);
    DataContainer(const DataContainer& r);
    ~DataContainer();                                                       
    HGLOBAL createHDrop();
    CustomData* findCustom(unsigned int cf, bool append=false);
    inline size_t customCount() const {return _customCount;}
    const CustomData* custom() const {return _custom;}
    inline const MyStringW& dir() const {return _dir;}
    inline size_t fileCount() const {return _count;}
    inline const MyStringW* files() const {return _files;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__DATACONT_H__
