/**
 * @file dataobj.cpp
 * The contains implementation of DataObject class
 *
 */

#include <shlobj.h>
#include <guiddef.h>
#include <myshptr.h>
#include <myunknown.h>
#include <shutils.h>
#include <utils.h>

#include "dataobj.h"
#include "enumfmt.h"
#include "datacont.h"

/**
 * Custom data object used during dragging operation.
 */
class DataObject : public Unknown, public IDataObject, public IAsyncOperation
{
private:
    DataContainer _data;
    CIDA* _shellData;
    size_t _shellDataSize;
    BOOL _async;
    BOOL _operating;
    HRESULT QueryGetCustomData(FORMATETC* fmt);
public:
    DataObject(const DataContainer& data): _data(data), _shellData(nullptr), _shellDataSize(0),
        _async(VARIANT_TRUE), _operating(VARIANT_FALSE){}
    ~DataObject()
    {
        if (_shellData)
            free(_shellData);
    }

    DEFINE_UNKNOWN

    HRESULT WINAPI QueryInterface (
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
    HRESULT WINAPI GetData(
            /* [unique][in] */ FORMATETC* /*pformatetcIn*/,
            /* [out] */ STGMEDIUM* /*pmedium*/);
    virtual HRESULT WINAPI GetDataHere(
        /* [unique][in] */ FORMATETC* /*pformatetc*/,
        /* [out][in] */ STGMEDIUM* /*pmedium*/)
    {
        NYI();
        return E_NOTIMPL;
    }

    HRESULT WINAPI QueryGetData(
        /* [unique][in] */ FORMATETC* /*pformatetc*/);

    virtual HRESULT WINAPI GetCanonicalFormatEtc(
        /* [unique][in] */ FORMATETC* /*pformatectIn*/,
        /* [out] */ FORMATETC* /*pformatetcOut*/)
    {
        NYI();
        return E_NOTIMPL;
    }

    HRESULT WINAPI SetData (
        /* [unique][in] */ FORMATETC* /*pformatetc*/,
        /* [unique][in] */ STGMEDIUM* /*pmedium*/,
        /* [in] */ BOOL /*fRelease*/);

    virtual HRESULT WINAPI DAdvise(
        /* [in] */ FORMATETC* /*pformatetc*/,
        /* [in] */ DWORD /*advf*/,
        /* [unique][in] */ IAdviseSink* /*pAdvSink*/,
        /* [out] */ DWORD* /*pdwConnection*/)
    {
        NYI();
        return E_NOTIMPL;
    }

    virtual HRESULT WINAPI DUnadvise(
        /* [in] */ DWORD /*dwConnection*/)
    {
        NYI();
        return E_NOTIMPL;
    }

    virtual HRESULT WINAPI EnumDAdvise(
        /* [out] */ IEnumSTATDATA** /*ppenumAdvise*/)
    {
        NYI();
        return E_NOTIMPL;
    }
    HRESULT WINAPI EnumFormatEtc (
    /* [in] */ DWORD dwDirection,
    /* [out] */ IEnumFORMATETC** ppenumFormatEtc);

    HRESULT WINAPI EndOperation(HRESULT hr, IBindCtx* /*pbc*/, DWORD dwEffects)
    {
        TRACE_FUNC();
        _operating = VARIANT_FALSE;
        if (SUCCEEDED(hr) && dwEffects == DROPEFFECT_MOVE)
            deleteFilesFromDataObj(this);
        return S_OK;
    }
    HRESULT WINAPI GetAsyncMode(BOOL* mode)
    {
        TRACE_FUNC();
        if (IsBadWritePtr(mode, sizeof(*mode)))
            return E_POINTER;
        *mode = _async;
        return S_OK;
    }
    HRESULT WINAPI InOperation(BOOL* value)
    {
        TRACE_FUNC();
        if (IsBadWritePtr(value, sizeof(*value)))
            return E_POINTER;
        *value = _operating;
        return S_OK;
    }
    HRESULT WINAPI SetAsyncMode(BOOL mode)
    {
        TRACE_FUNC();
        if (mode != VARIANT_TRUE && mode != VARIANT_FALSE)
            return E_INVALIDARG;
        _async = mode;
        return S_OK;
    }
    HRESULT WINAPI StartOperation(IBindCtx* /*bc*/)
    {
        TRACE_FUNC();
        _operating = VARIANT_TRUE;
        return S_OK;
    }
};

class ShellIDList
{
    ITEMIDLIST* _folderIL = nullptr;
    ShPtr<IShellFolder> _folder;
    ITEMIDLIST** _files = nullptr;
    UINT _numFiles = 0;
public:
    ~ShellIDList()
    {
        if (_files)
        {
            for (size_t i = _numFiles; i--; )
                ILFree(_files[i]);
            delete[] _files;
        }
        if (_folderIL)
            ILFree(_folderIL);
    }
    HRESULT createFromDataContainer(const DataContainer& data)
    {
        ASSERT(!_folderIL && !_files && !_numFiles);
        HRESULT res = S_OK;

        ShPtr<IShellFolder> desktop;
        res = SHGetDesktopFolder(&desktop);
        if (FAILED(res))
            return res;

        ULONG eaten;
        res = desktop->ParseDisplayName(NULL, NULL,
            const_cast<LPOLESTR>(static_cast<const wchar_t*>(data.dir())), &eaten, &_folderIL, NULL);
        if (FAILED(res))
            return res;

        res = desktop->BindToObject(_folderIL, NULL, IID_IShellFolder,
            reinterpret_cast<void**>(&_folder));

        if (FAILED(res))
            return res;

        _files = new ITEMIDLIST*[data.fileCount()];
        int i;
        for (i = 0; i < static_cast<int>(data.fileCount()); i++)
        {
            res = _folder->ParseDisplayName(NULL, NULL,
                const_cast<LPOLESTR>(static_cast<const wchar_t*>(data.files()[i])), &eaten, &_files[i], NULL);

            if (FAILED(res))
                return res;
            _numFiles++;
        }
        return S_OK;
    }
    HRESULT getShellUIObject(REFIID iid, void** dataObject)
    {
        return _folder->GetUIObjectOf(NULL, _numFiles,
            const_cast<LPCITEMIDLIST*>(_files), iid, NULL, dataObject);
    }
    CIDA* getShellIDArray(size_t* dataSize) const
    {
        if (!_numFiles)
            return NULL;
        UINT headerSize = sizeof(CIDA) + _numFiles * sizeof(UINT);
        size_t totalSize = headerSize;
        totalSize += ILGetSize(_folderIL);
        for (size_t i = 0; i < _numFiles; i++)
            totalSize += ILGetSize(_files[i]);
        *dataSize = totalSize;
        CIDA* data = reinterpret_cast<CIDA*>(malloc(totalSize));
        if (!data)
            return NULL;
        data->cidl = _numFiles;
        UINT offset = headerSize;
        {
            data->aoffset[0] = offset;
            UINT sz = ILGetSize(_folderIL);
            CopyMemory(reinterpret_cast<char*>(data) + offset, _folderIL, sz);
            offset += sz;
        }
        for (size_t i = 0; i < _numFiles; i++)
        {
            data->aoffset[1+i] = offset;
            UINT sz = ILGetSize(_files[i]);
            CopyMemory(reinterpret_cast<char*>(data) + offset, _files[i], sz);
            offset += sz;
        }
        ASSERT(offset == totalSize);
        return data;
    }
};

HRESULT getShellUIObject(const DataContainer& data, REFIID iid, void** dataObject)
{
    ShellIDList idList;
    HRESULT res = idList.createFromDataContainer(data);
    if (FAILED(res))
        return res;
    return idList.getShellUIObject(iid, dataObject);
}

/**
 * Creates DataObject from plug-in's panel info structure
 */
HRESULT createDataObject(const DataContainer& data, IDataObject** dataObject, bool shellObj)
{
    HRESULT res = S_OK;

    if (IsBadWritePtr(dataObject, sizeof(*dataObject)))
        return E_INVALIDARG;

    if (shellObj)
        return getShellUIObject(data, IID_IDataObject, reinterpret_cast<void**>(dataObject));

    *dataObject = new DataObject(data);

    if (!*dataObject)
    {
        return E_OUTOFMEMORY;
    }

    (*dataObject)->AddRef();

    return res;
}

void deleteFilesFromDataObj(IDataObject* dropSource)
{
    TRACE("deleting files requested\n");
    FORMATETC format = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {TYMED_NULL, nullptr, nullptr};
    HRESULT result = dropSource->GetData(&format, &medium);
    if (SUCCEEDED(result) && medium.hGlobal && GlobalSize(medium.hGlobal) >= sizeof(DROPFILES))
    {
        DROPFILES* data = reinterpret_cast<DROPFILES*>(GlobalLock(medium.hGlobal));
        if (data)
        {
            WCHAR* filename = reinterpret_cast<WCHAR*>(reinterpret_cast<char*>(data) + data->pFiles);
            while (*filename)
            {
                TRACE("deleting file %S", filename);
                DeleteFile(filename);
                filename += lstrlen(filename) + 1;
            }
            GlobalUnlock(medium.hGlobal);
        }
    }
    ReleaseStgMedium(&medium);
}

HRESULT DataObject::QueryInterface (
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void **ppvObject)
{
    if (IsBadWritePtr(ppvObject, sizeof(*ppvObject)))
        return E_POINTER;

    if (IsEqualGUID(riid, IID_IDataObject))
    {
        *ppvObject = static_cast<IDataObject*>(this);
        AddRef();
        return S_OK;
    }
    else if (IsEqualGUID(riid, IID_IAsyncOperation))
    {
        *ppvObject = static_cast<IAsyncOperation*>(this);
        AddRef();
        return S_OK;
    }
    else
        return Unknown::QueryInterface(riid, ppvObject);
}

HRESULT DataObject::EnumFormatEtc (
    /* [in] */ DWORD dwDirection,
    /* [out] */ IEnumFORMATETC** ppenumFormatEtc)
{
    if (IsBadWritePtr(ppenumFormatEtc, sizeof(*ppenumFormatEtc)))
        return E_POINTER;

    switch(dwDirection)
    {
    case DATADIR_SET:
        {
            TRACE("Getting enum for DATADIR_SET\n");
            *ppenumFormatEtc = static_cast<IEnumFORMATETC*>(new EnumFORMATETC(NULL, 0));
            return S_OK;
        }
    case DATADIR_GET:
        {
            TRACE("Getting enum for DATADIR_GET\n");
            FORMATETC fmt[] =
            {
                {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
                {CF_SHELLIDLIST, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
            };
            *ppenumFormatEtc =
                static_cast<IEnumFORMATETC*>(new EnumFORMATETC(fmt, LENGTH(fmt),
                            _data.custom(), _data.customCount()));
            return S_OK;
        }
    default:
        return E_INVALIDARG;
    }
}

HRESULT DataObject::QueryGetCustomData(FORMATETC* fmt)
{
    CustomData* p = _data.findCustom(fmt->cfFormat);

    if (!p)
        return DV_E_CLIPFORMAT;
    if (p->fmt.ptd != fmt->ptd)
        return DV_E_DVTARGETDEVICE;
    else if (p->fmt.dwAspect != fmt->dwAspect)
        return DV_E_DVASPECT;
    else if (p->fmt.lindex != fmt->lindex)
        return DV_E_LINDEX;
    else if (!(p->fmt.tymed & fmt->tymed))
        return DV_E_TYMED;

    return S_OK;
}

HRESULT DataObject::QueryGetData(
    /* [unique][in] */ FORMATETC* pformatetcIn)
{
    if (IsBadReadPtr(pformatetcIn, sizeof(*pformatetcIn)))
    {
        return E_POINTER;
    }

    HRESULT hr = QueryGetCustomData(pformatetcIn);

    if (SUCCEEDED(hr))
        return hr;

    if (pformatetcIn->dwAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;
    else if (pformatetcIn->lindex != -1 && pformatetcIn->lindex != 0)
        return DV_E_LINDEX;
    else if (pformatetcIn->ptd != NULL)
        return DV_E_DVTARGETDEVICE;
    else if (!(pformatetcIn->tymed & TYMED_HGLOBAL))
        return DV_E_TYMED;
    else if (pformatetcIn->cfFormat == CF_HDROP)
        return S_OK;
    else if (pformatetcIn->cfFormat == CF_SHELLIDLIST)
        return S_OK;
    else
        return DV_E_CLIPFORMAT;
}

HRESULT WINAPI DataObject::GetData(
        /* [unique][in] */ FORMATETC* pformatetcIn,
        /* [out] */ STGMEDIUM* pmedium)
{
#if 0
    TRACE("Getting data \"");
    TRACECBFMT(pformatetcIn->cfFormat);
    TRACE("\"\n");
#endif

    HRESULT res = QueryGetData(pformatetcIn);
    if (FAILED(res))
        return res;

    if (IsBadWritePtr(pmedium, sizeof(*pmedium)))
        return E_POINTER;

    if (pformatetcIn->cfFormat == CF_HDROP)
    {
        ZeroMemory(pmedium, sizeof(*pmedium));
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = _data.createHDrop();
        return S_OK;
    }
    else if (pformatetcIn->cfFormat == CF_SHELLIDLIST)
    {
        if (!_shellData)
        {
            ShellIDList list;
            HRESULT result = list.createFromDataContainer(_data);
            if (FAILED(result))
                return result;
            _shellData = list.getShellIDArray(&_shellDataSize);
            if (!_shellData)
                return E_OUTOFMEMORY;
        }
        ZeroMemory(pmedium, sizeof(*pmedium));
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = GlobalAlloc(GMEM_FIXED, _shellDataSize);
        if (!pmedium->hGlobal)
            return E_OUTOFMEMORY;
        CopyMemory(reinterpret_cast<CIDA*>(pmedium->hGlobal), _shellData, _shellDataSize);
        return S_OK;
    }
    else
    {
        if (pformatetcIn->cfFormat == CF_DRAGIMAGEBITS)
        {
            TRACE("Getting DragImageBits\n");
        }
        else if (pformatetcIn->cfFormat == CF_DRAGCONTEXT)
        {
            TRACE("Getting CF_DRAGCONTEXT\n");
        }
        CustomData* p = _data.findCustom(pformatetcIn->cfFormat);
        if (p)
        {
            CopyMedium(*pmedium, p->mdm, pformatetcIn->cfFormat);
            return S_OK;
        }
    }

    return DV_E_CLIPFORMAT;
}

HRESULT WINAPI DataObject::SetData (
    /* [unique][in] */ FORMATETC* pformatetc,
    /* [unique][in] */ STGMEDIUM* pmedium,
    /* [in] */ BOOL fRelease)
{
#if 0
    TRACE("Setting data, fReleas is %s \"", (fRelease?"TRUE":"FALSE"));
    TRACECBFMT(pformatetc->cfFormat);
    TRACE("\"\n");
#endif

    if (pformatetc->ptd != NULL)
        return DV_E_CLIPFORMAT;

    if (pformatetc->cfFormat == CF_PERFORMEDDROPEFFECT)
    {
        if (fRelease)
        {
            ReleaseStgMedium(pmedium);
            return S_OK;
        }
    }
    else
    {
        if (pformatetc->cfFormat == CF_DRAGIMAGEBITS)
        {
            TRACE("Setting DragImageBits\n");
        }
        else if (pformatetc->cfFormat == CF_DRAGCONTEXT)
        {
            TRACE("Setting CF_DRAGCONTEXT\n");
        }
        CustomData* p = _data.findCustom(pformatetc->cfFormat, true);
        if (p)
        {
            p->fmt = *pformatetc;

            if (p->mdm.hGlobal)
                ReleaseStgMedium(&p->mdm);

            if (fRelease)
            {
                p->mdm = *pmedium;
            }
            else
            {
                CopyMedium(p->mdm, *pmedium, pformatetc->cfFormat);
            }
        }
    }

    return S_OK;
}

// vim: set et ts=4 ai :

