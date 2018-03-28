

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Dec 30 19:14:18 2016
 */
/* Compiler settings for JCSrv.idl:
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

#ifndef __JCSrv_i_h__
#define __JCSrv_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICJfsSrv_FWD_DEFINED__
#define __ICJfsSrv_FWD_DEFINED__
typedef interface ICJfsSrv ICJfsSrv;
#endif 	/* __ICJfsSrv_FWD_DEFINED__ */


#ifndef __CJfsSrv_FWD_DEFINED__
#define __CJfsSrv_FWD_DEFINED__

#ifdef __cplusplus
typedef class CJfsSrv CJfsSrv;
#else
typedef struct CJfsSrv CJfsSrv;
#endif /* __cplusplus */

#endif 	/* __CJfsSrv_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ICJfsSrv_INTERFACE_DEFINED__
#define __ICJfsSrv_INTERFACE_DEFINED__

/* interface ICJfsSrv */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICJfsSrv;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DA69A941-4388-4735-8805-40FD33016B5C")
    ICJfsSrv : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ IUnknown *pCallBack,
            /* [in] */ DWORD dwPid) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UnInit( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetState( 
            /* [retval][out] */ DWORD *dwState) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Query( 
            /* [in] */ DWORD dwConditionMask,
            /* [in] */ BSTR lpstrIncludeDir,
            /* [in] */ BSTR lpstrExtension,
            /* [in] */ BSTR lpstrFileName) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetResultAt( 
            /* [in] */ DWORD dwIndex,
            /* [in] */ DWORD GetValueMask,
            /* [out] */ BSTR *szName,
            /* [out] */ BSTR *szPath,
            /* [out] */ BSTR *szExtension,
            /* [out] */ DWORD *dwLeave,
            /* [out] */ ULONGLONG *ullFileSize,
            /* [out] */ ULONGLONG *ullMondifyTime,
            /* [out] */ ULONGLONG *ullCreateTime,
            /* [out] */ ULONGLONG *ullAccessTime,
            /* [out] */ DWORD *dwFileAttr,
            /* [out] */ DWORD *dwIconIndex) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetFileMark( 
            /* [in] */ BSTR lpszFile,
            /* [in] */ DWORD dwLeave) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICJfsSrvVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICJfsSrv * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICJfsSrv * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICJfsSrv * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICJfsSrv * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICJfsSrv * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICJfsSrv * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICJfsSrv * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Init )( 
            ICJfsSrv * This,
            /* [in] */ IUnknown *pCallBack,
            /* [in] */ DWORD dwPid);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UnInit )( 
            ICJfsSrv * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetState )( 
            ICJfsSrv * This,
            /* [retval][out] */ DWORD *dwState);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Query )( 
            ICJfsSrv * This,
            /* [in] */ DWORD dwConditionMask,
            /* [in] */ BSTR lpstrIncludeDir,
            /* [in] */ BSTR lpstrExtension,
            /* [in] */ BSTR lpstrFileName);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetResultAt )( 
            ICJfsSrv * This,
            /* [in] */ DWORD dwIndex,
            /* [in] */ DWORD GetValueMask,
            /* [out] */ BSTR *szName,
            /* [out] */ BSTR *szPath,
            /* [out] */ BSTR *szExtension,
            /* [out] */ DWORD *dwLeave,
            /* [out] */ ULONGLONG *ullFileSize,
            /* [out] */ ULONGLONG *ullMondifyTime,
            /* [out] */ ULONGLONG *ullCreateTime,
            /* [out] */ ULONGLONG *ullAccessTime,
            /* [out] */ DWORD *dwFileAttr,
            /* [out] */ DWORD *dwIconIndex);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetFileMark )( 
            ICJfsSrv * This,
            /* [in] */ BSTR lpszFile,
            /* [in] */ DWORD dwLeave);
        
        END_INTERFACE
    } ICJfsSrvVtbl;

    interface ICJfsSrv
    {
        CONST_VTBL struct ICJfsSrvVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICJfsSrv_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICJfsSrv_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICJfsSrv_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICJfsSrv_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICJfsSrv_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICJfsSrv_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICJfsSrv_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICJfsSrv_Init(This,pCallBack,dwPid)	\
    ( (This)->lpVtbl -> Init(This,pCallBack,dwPid) ) 

#define ICJfsSrv_UnInit(This)	\
    ( (This)->lpVtbl -> UnInit(This) ) 

#define ICJfsSrv_GetState(This,dwState)	\
    ( (This)->lpVtbl -> GetState(This,dwState) ) 

#define ICJfsSrv_Query(This,dwConditionMask,lpstrIncludeDir,lpstrExtension,lpstrFileName)	\
    ( (This)->lpVtbl -> Query(This,dwConditionMask,lpstrIncludeDir,lpstrExtension,lpstrFileName) ) 

#define ICJfsSrv_GetResultAt(This,dwIndex,GetValueMask,szName,szPath,szExtension,dwLeave,ullFileSize,ullMondifyTime,ullCreateTime,ullAccessTime,dwFileAttr,dwIconIndex)	\
    ( (This)->lpVtbl -> GetResultAt(This,dwIndex,GetValueMask,szName,szPath,szExtension,dwLeave,ullFileSize,ullMondifyTime,ullCreateTime,ullAccessTime,dwFileAttr,dwIconIndex) ) 

#define ICJfsSrv_SetFileMark(This,lpszFile,dwLeave)	\
    ( (This)->lpVtbl -> SetFileMark(This,lpszFile,dwLeave) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICJfsSrv_INTERFACE_DEFINED__ */



#ifndef __JCSrvLib_LIBRARY_DEFINED__
#define __JCSrvLib_LIBRARY_DEFINED__

/* library JCSrvLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_JCSrvLib;

EXTERN_C const CLSID CLSID_CJfsSrv;

#ifdef __cplusplus

class DECLSPEC_UUID("E6F8B889-85AA-4BC1-B360-5E488FB8FEE9")
CJfsSrv;
#endif
#endif /* __JCSrvLib_LIBRARY_DEFINED__ */

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


