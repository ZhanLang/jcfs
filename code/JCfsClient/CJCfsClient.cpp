// CJCfsClient.cpp : CCJCfsClient 的实现

#include "stdafx.h"
#include "CJCfsClient.h"




STDMETHODIMP CCJCfsClient::Init(IDiskSearchNotify* pClientNotify)
{
	RASSERT(pClientNotify, E_INVALIDARG);

	m_pNotify = pClientNotify;
	RFAILED(CoCreateInstance(CLSID_CJfsSrv, NULL, CLSCTX_LOCAL_SERVER, IID_ICJfsSrv, (void**)&m_CJfsSrv));
	RASSERT( m_CJfsSrv, E_FAIL);
	RFAILED(m_CJfsSrv->Init((IUnknown*)((ICJCfsCallBack*)this),GetCurrentProcessId()));
	
	return S_OK;
}

STDMETHODIMP CCJCfsClient::UnInit()
{
	if ( m_CJfsSrv )
	{
		m_CJfsSrv->UnInit();
		m_CJfsSrv = INULL;
	}

	if ( m_pNotify )
	{
		m_pNotify = INULL;
	}
	return S_OK;
}




STDMETHODIMP CCJCfsClient::Query(QueryCondition& queryCondition)
{
	RASSERT(m_CJfsSrv, E_FAIL);
	return m_CJfsSrv->Query(queryCondition.dwConditionMask, 
		CComBSTR(queryCondition.lpstrIncludeDir),
		CComBSTR(queryCondition.lpstrExtension),
		CComBSTR(queryCondition.lpstrFileName));

}


static int GetFileIconIndex(PWCHAR pszFullPath)
{
	SHFILEINFOW sfi = { 0 };
	SHGetFileInfoW(pszFullPath,  
		FILE_ATTRIBUTE_NORMAL,  
		&sfi,  
		sizeof(sfi),  
		SHGFI_SYSICONINDEX 
		) ;

	return sfi.iIcon;
}

inline static int GetDirectoryIconIndex(PWCHAR pszPath,int pathLen) 
{      
	if(0==pathLen)
	{
		SHFILEINFOW sfi = { 0 };
		sfi.iIcon = -1;
		WCHAR path[MAX_PATH] = { 0 };      
		_tcscpy(path, pszPath);
		path[3]=L'\0';
		SHGetFileInfoW(path,  
			FILE_ATTRIBUTE_DIRECTORY,  
			&sfi, sizeof(sfi),  
			SHGFI_SYSICONINDEX );
		return sfi.iIcon;
	}else{
		static int dirIconIndex=-1;
		if(-1==dirIconIndex){
			SHFILEINFOW sfi = { 0 };
			WCHAR path[MAX_PATH] = { 0 };
			GetSystemDirectoryW(path,MAX_PATH);
			SHGetFileInfoW(path,  
				FILE_ATTRIBUTE_DIRECTORY,  
				&sfi,  sizeof(sfi),  
				SHGFI_SYSICONINDEX  );

			dirIconIndex=sfi.iIcon;              
		}
		return dirIconIndex;      
	}
}

STDMETHODIMP CCJCfsClient::GetResultAt(DWORD dwIndex, DWORD GetValueMask, SearchResult& ResultInfo)
{
	RASSERT(m_CJfsSrv, E_FAIL);

	CComBSTR szName, szPath, szExtension;
	DWORD dwLeave = 0;

	DWORD tempGetValueMask = GetValueMask & ~GetValueMask_IcoIndex;

	HRESULT hR = m_CJfsSrv->GetResultAt(dwIndex, tempGetValueMask, &szName, &szPath, &szExtension, &dwLeave,
		&ResultInfo.ullFileSize, 
		&ResultInfo.ullMondifyTime,
		&ResultInfo.ullCreateTime, &ResultInfo.ullAccessTime, &ResultInfo.dwFileAttr,&ResultInfo.dwIconIndex);
	ResultInfo.dwLeave = dwLeave;
	if ( szName )
	{
		wcscpy_s(ResultInfo.szName, _countof(ResultInfo.szName), (LPCWSTR)szName);
	}
	
	if ( szPath )
	{
		wcscpy_s(ResultInfo.szPath, _countof(ResultInfo.szPath), (LPCWSTR)szPath);
	}
	
	if ( szExtension )
	{
		wcscpy_s(ResultInfo.szExtension, _countof(ResultInfo.szExtension), (LPCWSTR)szExtension);
	}
	
// 	CString strFullPath; strFullPath.Format(_T("%s%s"), ResultInfo.szPath, ResultInfo.szName);
// 	if (ResultInfo.dwFileAttr & FILE_ATTRIBUTE_DIRECTORY )
// 	{
// 		ResultInfo.dwIconIndex = GetDirectoryIconIndex(strFullPath.GetBuffer(), strFullPath.GetLength());
// 	}
// 	else
// 	{
// 		ResultInfo.dwIconIndex = GetFileIconIndex(strFullPath.GetBuffer());
// 	}
// 	
// 	
// 
// 	return hR;
	if ( GetValueMask & GetValueMask_IcoIndex )
	{
		if ( ResultInfo.dwFileAttr & FILE_ATTRIBUTE_DIRECTORY )
		{
			if (_tcslen(ResultInfo.szPath) == 0 && _tcslen( ResultInfo.szName) == 2 && ResultInfo.szName[1] == ':' )//是盘符
			{
				if ( ResultInfo.szName[0] == 'C')//系统盘符
				{
					static INT gSysDirIcon = GetDirectoryIconIndex(_T("C:"),0);
					ResultInfo.dwIconIndex = gSysDirIcon;
				}
				else
				{
					static INT gRootDirIcon = GetDirectoryIconIndex(_T("D:"),0);
					ResultInfo.dwIconIndex = gRootDirIcon;
				}
			}
			else
			{
				static INT g_iDirIcon = GetDirectoryIconIndex(L"c:\\",3);
				ResultInfo.dwIconIndex = g_iDirIcon;
			}
		}
		else
		{
			CString strFullPath; strFullPath.Format(_T("%s%s"), ResultInfo.szPath, ResultInfo.szName);
			CString strExtension = ResultInfo.szExtension;

			if (
				strExtension.CompareNoCase(_T("lnk")) == 0 || 
				strExtension.CompareNoCase(_T("exe")) == 0
				)
			{
				ResultInfo.dwIconIndex = GetFileIconIndex(strFullPath.GetBuffer());
			}
			else
			{
				CIConIndex::iterator it = m_IConIndex.find(ResultInfo.szExtension);
				if( it != m_IConIndex.end())
					ResultInfo.dwIconIndex = it->second;
				else
				{
					ResultInfo.dwIconIndex = GetFileIconIndex(strFullPath.GetBuffer());
					m_IConIndex[ strExtension] = ResultInfo.dwIconIndex;
				}
			}
		}
	}
	

	
	return hR;
}

STDMETHODIMP_(DWORD) CCJCfsClient::GetState()
{
	RASSERT(m_CJfsSrv, DiskState_UnKnown);

	DWORD dwState = DiskState_UnKnown;
	m_CJfsSrv->GetState(&dwState);
	return dwState;
}

STDMETHODIMP CCJCfsClient::SetFileMark(LPCWSTR lpszFile, DWORD dwLeave)
{
	RASSERT(m_CJfsSrv, E_FAIL);

	return m_CJfsSrv->SetFileMark(CComBSTR(lpszFile), dwLeave);
}

STDMETHODIMP CCJCfsClient::OnDiskSearch_FileChange(WCHAR cDosName, BSTR lpFile, DWORD dwAction, DWORD dwAttr)
{
	if ( m_pNotify )
	{
		m_pNotify->OnDiskSearch_FileChange(cDosName, lpFile, dwAction, dwAttr);
	}
	return S_OK;
}


STDMETHODIMP CCJCfsClient::OnDiskSearch_Progress(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur)
{
	if ( m_pNotify )
	{
		m_pNotify->OnDiskSearch_Progress(cDosName, dwTotalFile, dwCur);
	}
	return S_OK;
}


STDMETHODIMP CCJCfsClient::OnDiskSearch_FileCountChange(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount)
{
	if ( m_pNotify )
	{
		m_pNotify->OnDiskSearch_FileCountChange(cDosName, dwFileCount, dwDirCount);
	}

	return S_OK;
}


STDMETHODIMP CCJCfsClient::OnDiskSearch_StateChangeNotify(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam)
{
	if ( m_pNotify)
	{
		m_pNotify->OnDiskSearch_StateChangeNotify(cDosName, nMsg, wParam, lParam);
	}

	return S_OK;
}


STDMETHODIMP CCJCfsClient::OnDiskSearch_Result(DWORD dwCount, DWORD dwTickCount)
{
	if ( m_pNotify )
	{
		m_pNotify->OnDiskSearch_Result(dwCount,dwTickCount);
	}

	return S_OK;
}
