#include "StdAfx.h"
#include "DiskSearchCli.h"
#include <msapi/mswinapi.h>
#include <set>
#include "fSearchDef.h"

CDiskSearchCli::CDiskSearchCli(void)
{
	m_hExit = NULL;
	m_hQueryEvent = NULL;
	m_hQueryThread = NULL;
}


CDiskSearchCli::~CDiskSearchCli(void)
{
}


STDMETHODIMP CDiskSearchCli::Init(IDiskSearchNotify* pClientNotify)
{
	RASSERT(pClientNotify, E_INVALIDARG);
	m_pFileSearchClientNotify = pClientNotify;

	if (FAILED(CreateQueryThread()))
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient::CreateQueryThread()"));
		return E_FAIL;
	}

	m_pRot->GetObject(CLSID_DiskSearchSrv, re_uuidof(IDiskSearchSrv), (void**)&m_pFileSearchSrv.m_p);
	if (!m_pFileSearchSrv)
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient::GetObject(CLSID_FileSearchSrv)"));
		return E_FAIL;
	}

	m_pRot->CreateInstance(CLSID_MsSafeDwordVector, NULL, re_uuidof(IMsDwordVector), (void**)&m_pResult.m_p);
	if (!m_pResult)
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient::CreateInstance(CLSID_MsSafeDwordVector)"));
		return E_FAIL;
	}

	//设置步长
	if (FAILED(m_pResult->SetStep(50000)))
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient::SetStep()"));
		return E_FAIL;
	}

	//获取所有对象
	UTIL::com_ptr<IProperty2> pDiskProp;
	DWORD dwCount = m_pFileSearchSrv->GetSearchDisk((IMSBase**)&pDiskProp.m_p);
	GrpMsg(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient GetSearchDisk(%d)"), dwCount);
	RASSERT(pDiskProp, E_FAIL);
	CPropSet propSet(pDiskProp);
	for (DWORD dwLoop = 0 ; dwLoop < dwCount ; dwLoop++)
	{
		WCHAR cDosName = propSet[dwLoop];
		GrpMsg(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient(%c)"), cDosName);
		HRESULT hr = CreateDiskObject(cDosName);
		if (FAILED(hr))
		{
			GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::InitFileSearchClient::CreateDiskObject(%c)"), cDosName);
		}
	}

	m_UseFileSearchSrvNotifyConnectPoint.Connect(m_pFileSearchSrv, UTIL::com_ptr<IMSBase>(this));

	return S_OK;
}

STDMETHODIMP CDiskSearchCli::UnInit()
{
	m_UseFileSearchSrvNotifyConnectPoint.DisConnect();
	DestoryQueryThread();
	std::set<WCHAR> diskSet;
	{
		AUTOLOCK_CS(m_diskMap);
		for (CDiskMap::iterator it = m_diskMap.begin(); it != m_diskMap.end()  ; it++)
		{
			diskSet.insert(it->first);
		}
	}
	

	for (std::set<WCHAR>::iterator itSet = diskSet.begin(); itSet != diskSet.end(); itSet++)
	{
		DestoryDiskObject(*itSet);
	}



	
	SAFE_RELEASE(m_pFileSearchSrv);
	SAFE_RELEASE(m_pRot);
	return S_OK;
}

STDMETHODIMP CDiskSearchCli::Query(QueryCondition& queryCondition)
{
	m_queryCondition = queryCondition;
	SetEvent(m_hQueryEvent); //查询
	return S_OK;
}

STDMETHODIMP CDiskSearchCli::GetResultAt(DWORD dwIndex, DWORD GetValueMask, SearchResult& ResultInfo)
{
	RASSERT(dwIndex < m_pResult->GetCount(), E_FAIL);
	DWORD dwResult = m_pResult->GetAt(dwIndex);
	
	//获取盘符号
	WCHAR cDosName = GetResultVolume(dwResult);

	AUTOLOCK_CS(m_diskMap);
	CDiskMap::iterator it = m_diskMap.find(cDosName);
	RASSERT(it != m_diskMap.end() , E_FAIL);

	
	UTIL::com_ptr<IDiskSearch> pDisk = it->second.pObject;
	RASSERT(pDisk, E_FAIL);

	HRESULT hr = pDisk->GetResultAt(dwResult, GetValueMask, ResultInfo);
	return hr;
}

STDMETHODIMP_(DWORD) CDiskSearchCli::GetState()
{
	return m_pFileSearchSrv->GetState();
}

STDMETHODIMP CDiskSearchCli::OnDiskSearch_FileChange(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr)
{
	CFuncTime funcTime(_T("CDiskSearchCli："), _T("OnDiskSearch_FileChange"));
	return m_pFileSearchClientNotify->OnDiskSearch_FileChange(cDosName, lpFile, dwAction, dwAttr);
}

STDMETHODIMP CDiskSearchCli::OnDiskSearch_Progress(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur)
{
	CFuncTime funcTime(_T("CDiskSearchCli："), _T("OnDiskSearch_Progress"));
	return m_pFileSearchClientNotify->OnDiskSearch_Progress(cDosName, dwTotalFile, dwCur);
}

STDMETHODIMP CDiskSearchCli::OnDiskSearch_FileCountChange(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount)
{
	CFuncTime funcTime(_T("CDiskSearchCli："), _T("OnDiskSearch_FileCountChange"));
	return m_pFileSearchClientNotify->OnDiskSearch_FileCountChange(cDosName, dwFileCount, dwDirCount);
}

STDMETHODIMP CDiskSearchCli::OnDiskSearch_StateChangeNotify(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam)
{
	CFuncTime funcTime(_T("CDiskSearchCli："), _T("OnDiskSearch_StateChangeNotify"));
	//这里是从服务组件上报上来的
	switch ( nMsg )
	{
	//一下是服务端使用的
	case DiskState_Mount:
		{
			CreateDiskObject(cDosName);
			break;
		}
	case DiskState_uMount:
		{
			DestoryDiskObject(cDosName);
			break;
		}
	}

	return m_pFileSearchClientNotify->OnDiskSearch_StateChangeNotify(cDosName, nMsg, wParam, lParam);
}


STDMETHODIMP CDiskSearchCli::OnDiskSearch_Result(DWORD dwCount, DWORD dwTickCount)
{
	return E_NOTIMPL;
}

HRESULT CDiskSearchCli::CreateQueryThread()
{
	if(NULL == m_hExit)
		m_hExit	= CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!m_hExit)
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::CreateQueryThread !m_hExit"));
		return E_FAIL;
	}

	if (NULL == m_hQueryEvent)
		m_hQueryEvent = CreateEvent(NULL, FALSE, FALSE,NULL); 

	if (!m_hQueryEvent)
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::CreateQueryThread !m_hQueryEvent"));
		return E_FAIL;
	}

	if (NULL == m_hQueryThread)
	{
		DWORD dwThreadId = 0;
		m_hQueryThread = (HANDLE)CreateThread(NULL, 0, _QueryThreadFunction, this, CREATE_SUSPENDED, &dwThreadId);
		if (!m_hQueryThread)
		{
			TCHAR sError[MAX_PATH] = {0};
			msapi::GetLastErrorText(sError, MAX_PATH);
			GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::CreateQueryThread !m_hQueryThread [%s]"), sError);
			return E_FAIL;
		}

		ResumeThread(m_hQueryThread);
		ResetEvent(m_hExit);
	}


	return (m_hExit && m_hQueryEvent && m_hQueryThread) ? S_OK : E_FAIL;
}


HRESULT CDiskSearchCli::DestoryQueryThread()
{
	if (m_hExit)
		SetEvent(m_hExit);	//通知退出

	if (m_hQueryThread)
		DWORD dwWait = WaitForSingleObject(m_hQueryThread, INFINITE);

	GrpMsg(GroupName, MsgLevel_Msg, _T("CFileSearchClient::DestoryQueryThread"));
	SAFE_CLOSEHANDLE(m_hQueryThread);
	SAFE_CLOSEHANDLE(m_hQueryEvent);
	SAFE_CLOSEHANDLE(m_hExit);

	return S_OK;
}

DWORD WINAPI CDiskSearchCli::_QueryThreadFunction(LPVOID lpParam)
{
	CDiskSearchCli* pThis = (CDiskSearchCli*)lpParam;
	return pThis->OnQuery();
}

struct SearchProInfo
{
	WCHAR cDocDevice;
	CDiskSearchCli* pThis;
};

DWORD WINAPI  _stdcall CDiskSearchCli::_ObjQueryThreadFunction(LPVOID lpParam)
{
	SearchProInfo* pSearchInfo = (SearchProInfo*)lpParam;
	CDiskSearchCli* pThis = pSearchInfo->pThis;
	WCHAR cDosName = pSearchInfo->cDocDevice;
	delete pSearchInfo;

	return pThis->OnClientQuery(cDosName);
}


HRESULT CDiskSearchCli::OnQuery()
{
	HANDLE hEvents[2] = {m_hExit, m_hQueryEvent};
	while (TRUE)
	{
		DWORD dwWait = WaitForMultipleObjects(_countof(hEvents), hEvents, FALSE, -1);
		DWORD dwTickCount = GetTickCount();
		switch(dwWait)
		{
		case WAIT_OBJECT_0://for m_hExit
			return 0;
		case WAIT_OBJECT_0+1://for m_hQueryEvent
			{
				m_pResult->Clear();
				HANDLE hFineshEvent[MAX_PATH] = { 0 };

				int nFineshEventSize = 0;
				{
					AUTOLOCK_CS(m_diskMap);
					for (CDiskMap::iterator it = m_diskMap.begin(); it != m_diskMap.end() ; it++)
					{
						DiskObject diskObject = it->second;
						hFineshEvent[nFineshEventSize] = diskObject.hFinish;
						nFineshEventSize++;
						Sleep(1);
						SetEvent(diskObject.hSearch);
					}
				}


				WaitForMultipleObjects(nFineshEventSize, hFineshEvent, TRUE, -1);

				dwTickCount = GetTickCount() - dwTickCount;
				GrpMsg(GroupName, MsgLevel_Msg, L"查询(%s:%d)一共用了(%d)毫秒\r\n",m_queryCondition.lpstrFileName,m_pResult->GetCount(),dwTickCount);
				if (m_pFileSearchClientNotify)
				{
					m_pFileSearchClientNotify->OnDiskSearch_Result(m_pResult->GetCount(),dwTickCount);
				}
			}
		}
	}
	return 0;
}

HRESULT CDiskSearchCli::OnClientQuery(WCHAR cDosName)
{
	GrpMsg(GroupName,MsgLevel_Msg, _T("CDiskSearchMap::Iterator it = m_diskMap.Find(%c)"), cDosName);
	DiskObject diskObject ;
	{
		AUTOLOCK_CS(m_diskMap);
		CDiskMap::iterator it = m_diskMap.find(cDosName);
		RASSERT( it != m_diskMap.end() , E_FAIL);
		diskObject = it->second;

	}

	HANDLE hEvents[2] = { diskObject.hExit, diskObject.hSearch };
	while (TRUE)
	{
		DWORD dwWait = WaitForMultipleObjects(_countof(hEvents), hEvents, FALSE, -1);
		switch (dwWait)
		{
		case WAIT_OBJECT_0://for m_hExit
			return 0;
		case WAIT_OBJECT_0 + 1://diskObject.hSearch
			{
				UTIL::com_ptr<IDiskSearch> pSearch = diskObject.pObject;
				if (pSearch)
				{
					if (FAILED(pSearch->Query(m_queryCondition, (IMsDwordVector*)m_pResult.m_p)))
					{
						GrpError(GroupName, MsgLevel_Error, _T("Query(%c) Faild"), cDosName);
					}
				}

				SetEvent(diskObject.hFinish);
			}
		}
	}
}


STDMETHODIMP CDiskSearchCli::SetFileMark(LPCWSTR lpszFile, DWORD dwLeave)
{
	RASSERT( m_pFileSearchSrv, E_FAIL);
	return m_pFileSearchSrv->SetFileMark(lpszFile, dwLeave);
}

HRESULT CDiskSearchCli::CreateDiskObject(WCHAR cDosName)
{
	GrpMsg(GroupName, MsgLevel_Msg, _T("CreateDiskObject(%c)"), cDosName);
	RASSERT(m_pFileSearchSrv, E_FAIL);

	{
		AUTOLOCK_CS(m_diskMap);
		CDiskMap::iterator it = m_diskMap.find(cDosName);
		if (it != m_diskMap.end())
		{
			GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::it == m_diskMap.end()[%c]"), cDosName);
			return S_FALSE;
		}
		//RASSERT(it == m_diskMap.end() , S_FALSE);
	}


	UTIL::com_ptr<IDiskSearch> pDisk;
	m_pFileSearchSrv->GetSearchDisk(cDosName, (IMSBase**)&pDisk.m_p);
	if (!pDisk)
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchClient::GetSearchDisk pDisk[%c]"), cDosName);
		return E_FAIL;
	}

	DiskObject diskObject;
	diskObject.hExit	= CreateEvent(NULL, FALSE, FALSE, NULL);
	RASSERT(diskObject.hExit, E_FAIL);

	diskObject.hFinish	= CreateEvent(NULL, FALSE, FALSE, NULL);
	RASSERT(diskObject.hFinish, E_FAIL);

	diskObject.hSearch	= CreateEvent(NULL, FALSE, FALSE, NULL);
	RASSERT(diskObject.hSearch, E_FAIL);

	diskObject.pObject	= pDisk;

	SearchProInfo* pSearchProInfo = new SearchProInfo();
	RASSERT(pSearchProInfo, E_FAIL);

	pSearchProInfo->cDocDevice = cDosName;
	pSearchProInfo->pThis = this;
	DWORD  dwThreadiId = 0;
	diskObject.hThread = (HANDLE)CreateThread(NULL, NULL, _ObjQueryThreadFunction, pSearchProInfo, CREATE_SUSPENDED, &dwThreadiId);
	RASSERT(diskObject.hThread, E_FAIL);
	{
		AUTOLOCK_CS(m_diskMap);
		m_diskMap[cDosName] = diskObject;
	}
	ResumeThread(diskObject.hThread);

	GrpMsg(GroupName, MsgLevel_Msg, _T("CFileSearchClient::GetSearchDisk(%c) pDisk"), cDosName);
	m_pFileSearchClientNotify->OnDiskSearch_StateChangeNotify(cDosName, DiskState_Mount, 0,0);



	//报告当前搜索对象的状态
	DWORD dwCurState = m_pFileSearchSrv->GetState();
	switch(dwCurState)
	{
	case DiskState_Scaning:
			m_pFileSearchClientNotify->OnDiskSearch_StateChangeNotify(0, DiskState_Scaning, 0,0);
			break;
	case DiskState_Finish:
		{
			m_pFileSearchClientNotify->OnDiskSearch_StateChangeNotify(0, DiskState_Finish, 0,0);
			//如果是完成状态的话报告文件总数
			DWORD dwFileCount = 0, dwDirCount = 0;
			diskObject.pObject->GetCount(&dwFileCount, &dwDirCount);
			m_pFileSearchClientNotify->OnDiskSearch_FileCountChange(cDosName, dwFileCount, dwDirCount);
			break;
		}
	}


	return S_OK;

}


HRESULT CDiskSearchCli::DestoryDiskObject(WCHAR cDosName)
{
	AUTOLOCK_CS(m_diskMap);
	if(cDosName>=L'a') cDosName-=32;//变大写
	CDiskMap::iterator it = m_diskMap.find(cDosName);
	RASSERT(it != m_diskMap.end() , S_OK);

	DiskObject diskObject = it->second;

	SetEvent(diskObject.hExit);
	WaitForSingleObject(diskObject.hThread, INFINITE);

	it->second.pObject = INULL;
	SAFE_CLOSEHANDLE(diskObject.hThread);
	SAFE_CLOSEHANDLE(diskObject.hExit);
	SAFE_CLOSEHANDLE(diskObject.hFinish);
	SAFE_CLOSEHANDLE(diskObject.hSearch);

	m_diskMap.erase(it);

	m_pFileSearchClientNotify->OnDiskSearch_StateChangeNotify(cDosName, DiskState_uMount, 0,0);
	
	DWORD dwFileCount = 0;
	DWORD dwDirCount = 0;

	for (CDiskMap::iterator it = m_diskMap.begin() ; it != m_diskMap.end() ; it++)
	{
		dwFileCount += it->second.dwFileCount;
		dwDirCount  += it->second.dwDirCount;
	}

	if (m_pFileSearchClientNotify)
	{
		m_pFileSearchClientNotify->OnDiskSearch_FileCountChange(0,dwFileCount, dwDirCount);
	}

	return S_OK;
}