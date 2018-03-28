

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Jul 20 17:18:34 2016
 */
/* Compiler settings for JCDeskBand.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __JCDeskBand_i_h__
#define __JCDeskBand_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICDeskBandExt_FWD_DEFINED__
#define __ICDeskBandExt_FWD_DEFINED__
typedef interface ICDeskBandExt ICDeskBandExt;
#endif 	/* __ICDeskBandExt_FWD_DEFINED__ */


#ifndef __CDeskBandExt_FWD_DEFINED__
#define __CDeskBandExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class CDeskBandExt CDeskBandExt;
#else
typedef struct CDeskBandExt CDeskBandExt;
#endif /* __cplusplus */

#endif 	/* __CDeskBandExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ICDeskBandExt_INTERFACE_DEFINED__
#define __ICDeskBandExt_INTERFACE_DEFINED__

/* interface ICDeskBandExt */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICDeskBandExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EF9D3032-F85B-4855-AE01-0071E511C587")
    ICDeskBandExt : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ICDeskBandExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICDeskBandExt * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICDeskBandExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICDeskBandExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICDeskBandExt * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICDeskBandExt * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICDeskBandExt * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICDeskBandExt * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } ICDeskBandExtVtbl;

    interface ICDeskBandExt
    {
        CONST_VTBL struct ICDeskBandExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICDeskBandExt_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICDeskBandExt_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICDeskBandExt_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICDeskBandExt_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICDeskBandExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICDeskBandExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICDeskBandExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICDeskBandExt_INTERFACE_DEFINED__ */



#ifndef __JCDeskBandLib_LIBRARY_DEFINED__
#define __JCDeskBandLib_LIBRARY_DEFINED__

/* library JCDeskBandLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_JCDeskBandLib;

EXTERN_C const CLSID CLSID_CDeskBandExt;

#ifdef __cplusplus

class DECLSPEC_UUID("AB821F6A-EEB2-470F-8CEE-433A1E54701A")
CDeskBandExt;
#endif
#endif /* __JCDeskBandLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


