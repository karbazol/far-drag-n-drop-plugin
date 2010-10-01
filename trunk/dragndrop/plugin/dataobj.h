#ifndef __KARBAZOL__DRAGNDROP_2_0__DATAOBJ_H__
#define __KARBAZOL__DRAGNDROP_2_0__DATAOBJ_H__

#include "far.h"
#include "datacont.h"

HRESULT getShellUIObject(const DataContainer& data, REFIID iid, void** dataObject);
HRESULT createDataObject(const DataContainer& data, IDataObject** dataObject, bool shellObj=false);

#endif // __KARBAZOL__DRAGNDROP_2_0__DATAOBJ_H__
