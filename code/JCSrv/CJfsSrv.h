// CJfsSrv.h : CCJfsSrv 的声明

#pragma once
#include "resource.h"       // 主符号



#include "JCSrv_i.h"
#include "..\JCfsClient\JCfsClient_i.h"
#include <process\CheckProcExit.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;

#include <IfSearch.h>
#include <json/json.h>
// CCJfsSrv

#define BEGIN_CALL_JSON_FUNC()\
		virtual HRESULT OnCallJsonFunction(LPCWSTR lpszFuncName, Json::Value Param, Json::Value& Result){

#define CALL_JSON_FUNC(funcName, func)\
	if ( _tcsicmp(lpszFuncName, funcName) == 0 )\
		return func(Param, Result);\

#define END_CALL_JSON_FUNC() return E_NOTIMPL;};

class ATL_NO_VTABLE CCJfsSrv :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCJfsSrv, &CLSID_CJfsSrv>,
	public IDispatchImpl<ICJfsSrv, &IID_ICJfsSrv, &LIBID_JCSrvLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDiskSearchNotify,
	public IProcExit
{
public:
	CCJfsSrv()
	{
		
		m_checkPid.SetCallback(this);
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CJFSSRV)


BEGIN_COM_MAP(CCJfsSrv)
	COM_INTERFACE_ENTRY(ICJfsSrv)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IDiskSearchNotify)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()


	HRESULT FinalConstruct();
	void FinalRelease();

protected://IDiskSearchNotify
	STDMETHOD(OnDiskSearch_FileChange)(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr);
	STDMETHOD(OnDiskSearch_Progress)(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur);
	STDMETHOD(OnDiskSearch_FileCountChange)(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount);
	STDMETHOD(OnDiskSearch_StateChangeNotify)(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(OnDiskSearch_Result)( DWORD dwCount, DWORD dwTickCount);
public:


private:
	CComQIPtr<ICJCfsCallBack> m_pCallBack;
	UTIL::com_ptr<IMscomRunningObjectTable> m_pRot;
	UTIL::com_ptr<IDiskSearchCli>		m_pFileSearch;
public:
	STDMETHOD(Init)(IUnknown* pCallBack, DWORD dwPid);
	STDMETHOD(UnInit)(void);

	STDMETHOD(GetState)(DWORD* dwState);
	STDMETHOD(Query)(DWORD dwConditionMask, BSTR lpstrIncludeDir, BSTR lpstrExtension, BSTR lpstrFileName);
	STDMETHOD(GetResultAt)(DWORD dwIndex, DWORD GetValueMask, BSTR* szName, BSTR* szPath, BSTR* szExtension, DWORD* dwLeave, ULONGLONG*	ullFileSize,
		ULONGLONG*	ullMondifyTime,
		ULONGLONG*	ullCreateTime,
		ULONGLONG*	ullAccessTime,
		DWORD*		dwFileAttr,
		DWORD*		dwIconIndex);
	STDMETHOD(SetFileMark)(BSTR lpszFile, DWORD dwLeave);

	virtual HRESULT OnAddProc(HANDLE hHandle, DWORD dwData);

	virtual HRESULT OnProcExit(HANDLE hHandle);

private:
	CCheckProcExit m_checkPid;
};

OBJECT_ENTRY_AUTO(__uuidof(CJfsSrv), CCJfsSrv)
