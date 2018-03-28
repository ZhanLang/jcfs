// CJfsSrv.cpp : CCJfsSrv 的实现

#include "stdafx.h"
#include "CJfsSrv.h"
#include <json/json.h>
// CCJfsSrv

HRESULT CCJfsSrv::FinalConstruct()
{
	m_checkPid.StartThread();
	return S_OK;
}

void CCJfsSrv::FinalRelease()
{
	UnInit();
	m_checkPid.StopThread();
	SAFE_RELEASE(m_pRot);
}



STDMETHODIMP CCJfsSrv::OnDiskSearch_FileChange(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr)
{
	if ( m_pCallBack )
	{
		m_pCallBack->OnDiskSearch_FileChange(cDosName, CComBSTR(lpFile), dwAction, dwAttr);
	}
	return S_OK;
}

STDMETHODIMP CCJfsSrv::OnDiskSearch_Progress(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur)
{
	if ( m_pCallBack )
	{
		m_pCallBack->OnDiskSearch_Progress(cDosName, dwTotalFile, dwCur);
	}
	return S_OK;
}

STDMETHODIMP CCJfsSrv::OnDiskSearch_FileCountChange(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount)
{
	if ( m_pCallBack )
	{
		m_pCallBack->OnDiskSearch_FileCountChange(cDosName, dwFileCount, dwDirCount);
	}
	return S_OK;
}

STDMETHODIMP CCJfsSrv::OnDiskSearch_StateChangeNotify(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam)
{
	if ( m_pCallBack )
	{
		m_pCallBack->OnDiskSearch_StateChangeNotify(cDosName, nMsg, wParam, lParam);
	}
	return S_OK;
}

STDMETHODIMP CCJfsSrv::OnDiskSearch_Result( DWORD dwCount, DWORD dwTickCount)
{
	if ( m_pCallBack )
	{
		m_pCallBack->OnDiskSearch_Result(dwCount,dwTickCount);
	}
	return S_OK;
}



STDMETHODIMP CCJfsSrv::Init(IUnknown* pCallBack, DWORD dwPid)
{
	// TODO: 在此添加实现代码
	GrpMsg(GroupName, MsgLevel_Msg, _T("初始化RPC服务端代理:%d"), dwPid);
	RASSERT(_AtlModule.m_pEnvMgr, E_FAIL);
	_AtlModule.m_pEnvMgr->GetRunningObjectTable(&m_pRot.m_p);
	RASSERT(m_pRot, E_FAIL);
	RASSERT(pCallBack, E_INVALIDARG);

	if ( pCallBack )
		m_pCallBack = pCallBack;

	


	if( FAILED(m_pRot->CreateInstance(CLSID_DiskSearchCli, NULL, __uuidof(IDiskSearchCli), (void**)&m_pFileSearch)) ) 
	{
		GrpError(GroupName, MsgLevel_Error, _T("CreateInstance CLSID_FileSearchClient "));
		return E_FAIL;
	}

	if( ! m_pFileSearch )
	{
		GrpError(GroupName, MsgLevel_Error, _T("FinalConstruct --> 创建客户端失败 "));
		return E_FAIL;
	}

	if ( FAILED( m_pFileSearch->Init(this) ) )
	{
		GrpError(GroupName, MsgLevel_Error, _T("FinalConstruct --> 创建客户端失败 "));
		return E_FAIL;
	}

	m_checkPid.AddProc(dwPid, (DWORD)this);
	return S_OK;


}
STDMETHODIMP CCJfsSrv::UnInit(void)
{
	// TODO: 在此添加实现代码

	if ( m_pCallBack )
	{
		m_pCallBack = INULL;
	}

	if ( m_pFileSearch )
	{
		m_pFileSearch->UnInit();
		m_pFileSearch = INULL;
	}

	return S_OK;
}

STDMETHODIMP CCJfsSrv::GetState(DWORD* dwState)
{
	RASSERT( dwState , E_INVALIDARG);
	RASSERT( m_pFileSearch, E_FAIL);
	*dwState = m_pFileSearch->GetState();
	return S_OK;
}


STDMETHODIMP CCJfsSrv::Query(DWORD dwConditionMask, BSTR lpstrIncludeDir, BSTR lpstrExtension, BSTR lpstrFileName)
{
	// TODO: 在此添加实现代码
	RASSERT( m_pFileSearch, E_FAIL);

	QueryCondition Query;
	Query.dwSize = sizeof( Query );
	Query.dwConditionMask = dwConditionMask;
	Query.lpstrExtension = lpstrExtension;
	Query.lpstrFileName = lpstrFileName;
	Query.lpstrIncludeDir = lpstrIncludeDir;

	return m_pFileSearch->Query(Query);
}


STDMETHODIMP CCJfsSrv::GetResultAt(DWORD dwIndex, DWORD GetValueMask, BSTR* szName, BSTR* szPath, BSTR* szExtension, DWORD* dwLeave,	
	ULONGLONG*	ullFileSize,
	ULONGLONG*	ullMondifyTime,
	ULONGLONG*	ullCreateTime,
	ULONGLONG*	ullAccessTime,
	DWORD*		dwFileAttr,
	DWORD*		dwIconIndex)
{
	// TODO: 在此添加实现代码
	RASSERT( m_pFileSearch, E_FAIL);

	SearchResult Result = { 0 };
	RFAILED(m_pFileSearch->GetResultAt(dwIndex, GetValueMask, Result));

	if ( szName )
	{
		CComBSTR bstrName(Result.szName); 
		bstrName.CopyTo(szName);
	}

	if ( szPath )
	{
		CComBSTR bstrPath(Result.szPath); 
		bstrPath.CopyTo(szPath);
	}

	if ( szExtension )
	{
		CComBSTR bstrExten(Result.szExtension); 
		bstrExten.CopyTo(szExtension);
	}
	
	if ( dwLeave )
	{
		*dwLeave = Result.dwLeave;
	}
	

	if (ullFileSize)	*ullFileSize = Result.ullFileSize;
	if (ullMondifyTime) *ullMondifyTime = Result.ullMondifyTime;
	if (ullCreateTime)	*ullCreateTime= Result.ullCreateTime;
	if (ullAccessTime)	*ullAccessTime= Result.ullAccessTime;
	if (dwFileAttr)		*dwFileAttr= Result.dwFileAttr;
	if ( dwIconIndex)	*dwIconIndex = Result.dwIconIndex;

	return S_OK;
}


STDMETHODIMP CCJfsSrv::SetFileMark(BSTR lpszFile, DWORD dwLeave)
{
	// TODO: 在此添加实现代码
	RASSERT( m_pFileSearch, E_FAIL);
	return m_pFileSearch->SetFileMark(lpszFile, dwLeave);
}

HRESULT CCJfsSrv::OnAddProc(HANDLE hHandle, DWORD dwData)
{
	return S_OK;
}

HRESULT CCJfsSrv::OnProcExit(HANDLE hHandle)
{
	UnInit();
	m_checkPid.StopThread();
	return S_OK;
}
