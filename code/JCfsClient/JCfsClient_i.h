

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Thu Jan 05 15:22:17 2017
 */
/* Compiler settings for JCfsClient.idl:
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

#ifndef __JCfsClient_i_h__
#define __JCfsClient_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICJCfsCallBack_FWD_DEFINED__
#define __ICJCfsCallBack_FWD_DEFINED__
typedef interface ICJCfsCallBack ICJCfsCallBack;
#endif 	/* __ICJCfsCallBack_FWD_DEFINED__ */


#ifndef __CJCfsClient_FWD_DEFINED__
#define __CJCfsClient_FWD_DEFINED__

#ifdef __cplusplus
typedef class CJCfsClient CJCfsClient;
#else
typedef struct CJCfsClient CJCfsClient;
#endif /* __cplusplus */

#endif 	/* __CJCfsClient_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ICJCfsCallBack_INTERFACE_DEFINED__
#define __ICJCfsCallBack_INTERFACE_DEFINED__

/* interface ICJCfsCallBack */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICJCfsCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("33A4E1E6-6AFA-43C2-AC14-599F33400A29")
    ICJCfsCallBack : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnDiskSearch_FileChange( 
            /* [in] */ WCHAR cDosName,
            /* [in] */ BSTR lpFile,
            /* [in] */ DWORD dwAction,
            /* [in] */ DWORD dwAttr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnDiskSearch_Progress( 
            /* [in] */ WCHAR cDosName,
            /* [in] */ DWORD dwTotalFile,
            /* [in] */ DWORD dwCur) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnDiskSearch_FileCountChange( 
            /* [in] */ WCHAR cDosName,
            /* [in] */ DWORD dwFileCount,
            /* [in] */ DWORD dwDirCount) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnDiskSearch_StateChangeNotify( 
            /* [in] */ WCHAR cDosName,
            /* [in] */ INT nMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnDiskSearch_Result( 
            /* [in] */ DWORD dwCount,
            /* [in] */ DWORD dwTickCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICJCfsCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICJCfsCallBack * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICJCfsCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICJCfsCallBack * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICJCfsCallBack * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICJCfsCallBack * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICJCfsCallBack * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICJCfsCallBack * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnDiskSearch_FileChange )( 
            ICJCfsCallBack * This,
            /* [in] */ WCHAR cDosName,
            /* [in] */ BSTR lpFile,
            /* [in] */ DWORD dwAction,
            /* [in] */ DWORD dwAttr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnDiskSearch_Progress )( 
            ICJCfsCallBack * This,
            /* [in] */ WCHAR cDosName,
            /* [in] */ DWORD dwTotalFile,
            /* [in] */ DWORD dwCur);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnDiskSearch_FileCountChange )( 
            ICJCfsCallBack * This,
            /* [in] */ WCHAR cDosName,
            /* [in] */ DWORD dwFileCount,
            /* [in] */ DWORD dwDirCount);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnDiskSearch_StateChangeNotify )( 
            ICJCfsCallBack * This,
            /* [in] */ WCHAR cDosName,
            /* [in] */ INT nMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnDiskSearch_Result )( 
            ICJCfsCallBack * This,
            /* [in] */ DWORD dwCount,
            /* [in] */ DWORD dwTickCount);
        
        END_INTERFACE
    } ICJCfsCallBackVtbl;

    interface ICJCfsCallBack
    {
        CONST_VTBL struct ICJCfsCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICJCfsCallBack_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICJCfsCallBack_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICJCfsCallBack_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICJCfsCallBack_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICJCfsCallBack_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICJCfsCallBack_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICJCfsCallBack_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICJCfsCallBack_OnDiskSearch_FileChange(This,cDosName,lpFile,dwAction,dwAttr)	\
    ( (This)->lpVtbl -> OnDiskSearch_FileChange(This,cDosName,lpFile,dwAction,dwAttr) ) 

#define ICJCfsCallBack_OnDiskSearch_Progress(This,cDosName,dwTotalFile,dwCur)	\
    ( (This)->lpVtbl -> OnDiskSearch_Progress(This,cDosName,dwTotalFile,dwCur) ) 

#define ICJCfsCallBack_OnDiskSearch_FileCountChange(This,cDosName,dwFileCount,dwDirCount)	\
    ( (This)->lpVtbl -> OnDiskSearch_FileCountChange(This,cDosName,dwFileCount,dwDirCount) ) 

#define ICJCfsCallBack_OnDiskSearch_StateChangeNotify(This,cDosName,nMsg,wParam,lParam)	\
    ( (This)->lpVtbl -> OnDiskSearch_StateChangeNotify(This,cDosName,nMsg,wParam,lParam) ) 

#define ICJCfsCallBack_OnDiskSearch_Result(This,dwCount,dwTickCount)	\
    ( (This)->lpVtbl -> OnDiskSearch_Result(This,dwCount,dwTickCount) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICJCfsCallBack_INTERFACE_DEFINED__ */



#ifndef __JCfsClientLib_LIBRARY_DEFINED__
#define __JCfsClientLib_LIBRARY_DEFINED__

/* library JCfsClientLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_JCfsClientLib;

EXTERN_C const CLSID CLSID_CJCfsClient;

#ifdef __cplusplus

class DECLSPEC_UUID("505E90B5-222A-4C5D-94B9-0FE39621503D")
CJCfsClient;
#endif
#endif /* __JCfsClientLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


