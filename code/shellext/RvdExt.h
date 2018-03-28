

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Dec 30 16:43:12 2016
 */
/* Compiler settings for RvdExt.idl:
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

#ifndef __RvdExt_h__
#define __RvdExt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IJCfsShlExt_FWD_DEFINED__
#define __IJCfsShlExt_FWD_DEFINED__
typedef interface IJCfsShlExt IJCfsShlExt;
#endif 	/* __IJCfsShlExt_FWD_DEFINED__ */


#ifndef __JCfsShlExt_FWD_DEFINED__
#define __JCfsShlExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class JCfsShlExt JCfsShlExt;
#else
typedef struct JCfsShlExt JCfsShlExt;
#endif /* __cplusplus */

#endif 	/* __JCfsShlExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IJCfsShlExt_INTERFACE_DEFINED__
#define __IJCfsShlExt_INTERFACE_DEFINED__

/* interface IJCfsShlExt */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IJCfsShlExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0ED235C1-3E4F-46DD-B8FC-80466731828A")
    IJCfsShlExt : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IJCfsShlExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IJCfsShlExt * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IJCfsShlExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IJCfsShlExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IJCfsShlExt * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IJCfsShlExt * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IJCfsShlExt * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IJCfsShlExt * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IJCfsShlExtVtbl;

    interface IJCfsShlExt
    {
        CONST_VTBL struct IJCfsShlExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJCfsShlExt_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IJCfsShlExt_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IJCfsShlExt_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IJCfsShlExt_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IJCfsShlExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IJCfsShlExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IJCfsShlExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IJCfsShlExt_INTERFACE_DEFINED__ */



#ifndef __JCFSEXTLib_LIBRARY_DEFINED__
#define __JCFSEXTLib_LIBRARY_DEFINED__

/* library JCFSEXTLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_JCFSEXTLib;

EXTERN_C const CLSID CLSID_JCfsShlExt;

#ifdef __cplusplus

class DECLSPEC_UUID("302B2B4E-FC21-4F87-BA10-5E286C3C5767")
JCfsShlExt;
#endif
#endif /* __JCFSEXTLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


