/**
 * @file dataobj.cpp
 * The contains implementation of DataObject class
 *
 * $Id: dataobj.cpp 78 2008-11-01 16:57:30Z eleskine $
 */

#include <shlobj.h>
#include <guiddef.h>
#include "utils.h"
#include "myunknown.h"
#include "dataobj.h"
#include "enumfmt.h"
#include "datacont.h"
#include "shutils.h"

/**
 * Custom data object used during dragging operation.
 */
class DataObject : public Unknown, public IDataObject, public IAsyncOperation
{
private:
    DataContainer _data;
    BOOL _async;
    BOOL _operating;
    HRESULT QueryGetCustomData(FORMATETC* fmt);
public:
    DataObject(const PanelInfoW& info): _data(info), _async(VARIANT_TRUE),
    _operating(VARIANT_FALSE){}
    
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

    HRESULT WINAPI EndOperation(HRESULT /*hr*/, IBindCtx* /*pbc*/, DWORD /*dwEffects*/)
    {
        TRACE_FUNC();
        _operating = VARIANT_FALSE;
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

/**
 * Creates DataObject from plug-in's panel info structure
 */
HRESULT createDataObject(PanelInfoW& info, IDataObject** dataObject)
{
    HRESULT res = S_OK;

    if (IsBadWritePtr(dataObject, sizeof(*dataObject)))
        return E_INVALIDARG;

    *dataObject = new DataObject(info);

    (*dataObject)->AddRef();

    return res;
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
            static FORMATETC fmt[] =
            {
                {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
                /*{CF_SHELLIDLIST, NULL, DVASPECT_CONTENT, -1 , TYMED_HGLOBAL},*/
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
    else if(pformatetcIn->lindex != -1 && pformatetcIn->lindex != 0)
        return DV_E_LINDEX;
    else if(pformatetcIn->ptd != NULL)
        return DV_E_DVTARGETDEVICE;
    else if(!(pformatetcIn->tymed & TYMED_HGLOBAL))
        return DV_E_TYMED;
    else if(pformatetcIn->cfFormat == CF_HDROP)
        return S_OK;
    /** @todo Implement support for CF_SHELLIDLIST */
#if 0
    else if(pformatetcIn->cfFormat == CF_SHELLIDLIST)
        return S_OK;
#endif
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
        p->fmt = *pformatetc;

        if (p->mdm.hGlobal)
            ReleaseStgMedium(&p->mdm);

        CopyMedium(p->mdm, *pmedium, pformatetc->cfFormat);
        if (fRelease)
            ReleaseStgMedium(pmedium);
    }

    return S_OK;
}

// vim: set et ts=4 ai :

