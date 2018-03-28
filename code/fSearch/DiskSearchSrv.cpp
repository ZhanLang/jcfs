#include "StdAfx.h"
#include "DiskSearchSrv.h"
#include <msapi/msapp.h>
#define CHECK_INDEX_INVALUDE _T("CHECK_INDEX_INVALUDE")
CDiskSearchSrv::CDiskSearchSrv(void)
{
	m_bReBuild = FALSE;
}


CDiskSearchSrv::~CDiskSearchSrv(void)
{
}


STDMETHODIMP CDiskSearchSrv::Init(void*)
{
	{
		TCHAR strValue[MAX_PATH] = { 0 };
		m_bootValue.GetVaule(CHECK_INDEX_INVALUDE, 0, strValue, MAX_PATH);
		INT nCount = _ttoi(strValue);

		CString strCount;
		strCount.Format(_T("%d"), ++nCount);
		m_bootValue.SetValue(CHECK_INDEX_INVALUDE, strCount);
	}


	CreateSqliteDB();
	AddConnectionPoint(re_uuidof(IDiskSearchNotify),m_FileSearchSrvNotifyConnectPoint);

	UTIL::com_ptr<IMSBase> m_diskMonitor;
	RFAILED(m_pRot->GetObject(CLSID_DiskMonitor,re_uuidof(IMSBase),(void**)&m_diskMonitor));
	RASSERT(m_diskMonitor,E_FAIL);
	RFAILED(m_useDiskChangePoint.Connect(m_diskMonitor,UTIL::com_ptr<IMSBase>(this)));


	TCHAR lpDriveString[MAX_PATH] = {0};
	GetLogicalDriveStrings(MAX_PATH,lpDriveString);
	int nCount  = 0;  
	TCHAR * pDrive = lpDriveString;  
	for(int nlen =_tcslen(lpDriveString); nlen == 3 ;nCount++)  
	{   
		WCHAR cDocName = pDrive[0];
		if(cDocName>=L'a') cDocName-=32;

		if (m_diskMap.find(cDocName) == m_diskMap.end() && GetDiskClsid(cDocName))
		{
			HRESULT hr = CreateDiskSearch(cDocName);
			if (FAILED(hr))
			{

			}
		}

		pDrive +=4;  
		nlen = _tcslen(pDrive);  
	}  


	return S_OK;
}


STDMETHODIMP CDiskSearchSrv::Uninit()
{
	DestorySqliteDB();
	DelConnectionPoint(re_uuidof(IDiskSearchNotify),m_FileSearchSrvNotifyConnectPoint);

	m_useDiskChangePoint.DisConnect();

	AUTOLOCK_CS(disk_map);
	for (CDiskMap::iterator it = m_diskMap.begin(); it != m_diskMap.end()  ; it++)
	{
		UTIL::com_ptr<IDiskSearch> pSearch = it->second;
		pSearch->CloseDisk();
		it->second = INULL;
	}
	m_diskMap.clear();

	SAFE_RELEASE(m_pRot);

	CString strCount;
	strCount.Format(_T("%d"), 0);
	m_bootValue.SetValue(CHECK_INDEX_INVALUDE, strCount);

	return S_OK;
}

STDMETHODIMP CDiskSearchSrv::Start()
{
	for (CDiskMap::iterator it = m_diskMap.begin(); it != m_diskMap.end()  ; )
	{
		UTIL::com_ptr<IDiskSearch> pSearch = it->second;
		HRESULT hr = pSearch->LoadDisk();
		if (FAILED(hr))
		{
			pSearch->CloseDisk();
			it = m_diskMap.erase(it);
		}
		else
		{
			it++;
		}
	}

	return S_OK;
}

STDMETHODIMP CDiskSearchSrv::Stop()
{

	CDiskMap disk;
	{
		AUTOLOCK_CS(disk_map);
		disk = m_diskMap;
	}
	
	
	for (CDiskMap::iterator it = disk.begin(); it != disk.end()  ; it++)
	{
		UTIL::com_ptr<IDiskSearch> pSearch = it->second;
		pSearch->UnLoadDisk();
	}

	return S_OK;
}


STDMETHODIMP CDiskSearchSrv::OnDiskChange(DWORD dwCtrlMask,DWORD dwType,LPCWSTR lpName)
{
	UTIL::com_ptr<IDiskSearch> pDiskSearch;
	WCHAR cDocName = lpName[0];
	if(cDocName>=L'a') cDocName-=32;

	switch(dwCtrlMask)
	{
	case DiskCtrl_Ready/*DiskCtrl_Mount*/:
		{
			HRESULT hr = CreateDiskSearch(cDocName);
			if (FAILED(hr))
			{

				break;
			}

			AUTOLOCK_CS(disk_map);
			CDiskMap::iterator it = m_diskMap.find(cDocName);
			if (it != m_diskMap.end())
			{
				UTIL::com_ptr<IDiskSearch> pDisk = it->second;
				if (pDisk)
				{
					hr = pDisk->LoadDisk();
					if (FAILED(hr))
					{
						pDisk->UnLoadDisk();
						pDisk->CloseDisk();
						m_diskMap.erase(it);
					}
					else
					{
						CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
							IDiskSearchNotify,
							OnDiskSearch_StateChangeNotify(cDocName, DiskState_Mount,0,0));
					}
				}
			}
			break;
		}
	case DiskCtrl_uMountQuery: //不释放是为了效率考虑，如果失败了不至于重新扫描一次
		{
			AUTOLOCK_CS(disk_map);
			CDiskMap::iterator it = m_diskMap.find(cDocName);
			if (it != m_diskMap.end())
			{
				UTIL::com_ptr<IDiskSearch> pDisk = it->second;
				if (pDisk)
				{
					pDisk->LockDisk();
				}
			}
			break;
		}
	case DiskCtrl_uMountFailed:
		{
			AUTOLOCK_CS(disk_map);
			CDiskMap::iterator it = m_diskMap.find(cDocName);
			if (it != m_diskMap.end())
			{
				UTIL::com_ptr<IDiskSearch> pDisk = it->second;
				if (pDisk)
				{
					pDisk->UnLockDisk();
				}
			}
			break;
		}
	case DiskCtrl_uMount:
		{
			CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
				IDiskSearchNotify,
				OnDiskSearch_StateChangeNotify(cDocName, DiskCtrl_uMount,0,0));

			DeleteDiskSearch(cDocName);
			break;
		}
	}
	return S_OK;
}


STDMETHODIMP CDiskSearchSrv::OnDiskSearch_FileChange(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr)
{
	CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
		IDiskSearchNotify,
		OnDiskSearch_FileChange(cDosName, lpFile,dwAction, dwAttr));

	return S_OK;
}

STDMETHODIMP CDiskSearchSrv::OnDiskSearch_Progress(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur)
{
	//汇总总进度

	DWORD dwAllTotalFile = 0, dwAllCurFile = 0;
	{
		AUTOLOCK_CS(m_FileProgressCacheMap);
		m_FileProgressCacheMap[cDosName].dwTotalCount = dwTotalFile;
		m_FileProgressCacheMap[cDosName].dwCurrentCount = dwCur;
		for ( CFileProgressCacheMap::iterator it = m_FileProgressCacheMap.begin() ; it != m_FileProgressCacheMap.end() ; it++)
		{
			dwAllTotalFile += it->second.dwTotalCount;
			dwAllCurFile += it->second.dwCurrentCount;
		}
	}

	CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
		IDiskSearchNotify,
		OnDiskSearch_Progress(0, dwAllTotalFile,dwAllCurFile));

	return S_OK;
}

STDMETHODIMP CDiskSearchSrv::OnDiskSearch_FileCountChange(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount)
{
	//汇总进度
	DWORD dwTotalFileCount = 0, dwTotalDirCount = 0;
	{
		AUTOLOCK_CS( m_FileCacheMap );
		m_FileCacheMap[cDosName].dwDirCount = dwDirCount;
		m_FileCacheMap[cDosName].dwFileCount = dwFileCount;
		for ( CFileCountCacheMap::iterator it = m_FileCacheMap.begin() ; it != m_FileCacheMap.end() ; it++)
		{
			dwTotalFileCount += it->second.dwFileCount;
			dwTotalDirCount += it->second.dwDirCount;
		}
	}


	CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
		IDiskSearchNotify,
		OnDiskSearch_FileCountChange(0, dwTotalFileCount,dwTotalDirCount));

	return S_OK;
}

STDMETHODIMP CDiskSearchSrv::OnDiskSearch_StateChangeNotify(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch( nMsg )
	{
	case DiskState_Scaning:	//
		{
			
			{//汇总结果
				AUTOLOCK_CS(m_DiskStateCacheMap);
				m_DiskStateCacheMap[cDosName].dwState = DiskState_Scaning;
			}
			
			//只要有一个还在扫描中就报扫描中
			CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
				IDiskSearchNotify,
				OnDiskSearch_StateChangeNotify(0, nMsg ,wParam, lParam));

			break;
		}
	case DiskState_Finish://
		{
			UTIL::com_ptr<IDiskSearch> pDiskSearch;
			GetSearchDisk(cDosName, (IMSBase**)&pDiskSearch);
			if (pDiskSearch)
				InitFileMark(cDosName,pDiskSearch);


			BOOL bFinish = TRUE;
			{//汇总结果
				AUTOLOCK_CS(m_DiskStateCacheMap);
				m_DiskStateCacheMap[cDosName].dwState = DiskState_Finish;

				for ( CDiskStateCacheMap::iterator it = m_DiskStateCacheMap.begin() ; it != m_DiskStateCacheMap.end() ; it++)
				{
					if ( it->second.dwState == DiskState_Scaning)
						bFinish = FALSE;
				}
			}
			

			//只有全部完成时才修改状态
			if ( bFinish )
			{
				CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
					IDiskSearchNotify,
					OnDiskSearch_StateChangeNotify(0, nMsg ,wParam, lParam));
			}


			GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]加载成功"), cDosName);

			break;
		}
	case DiskState_Failed://
		{
			//通知卸载
			CallConnectPointFunc(m_FileSearchSrvNotifyConnectPoint,
				IDiskSearchNotify,
				OnDiskSearch_StateChangeNotify(cDosName, DiskCtrl_uMount,0,0));

			DeleteDiskSearch(cDosName);
			break;//
		}
	}
	
	


	return E_NOTIMPL;
}

STDMETHODIMP CDiskSearchSrv::OnDiskSearch_Result(DWORD dwCount, DWORD dwTickCount)
{
	return E_NOTIMPL;
}

/*
STDMETHODIMP CDiskSearchSrv::OnStateChangeNotify(WCHAR cDosDrive, INT nMsg, WPARAM wParam, LPARAM lParam)
{
	
	switch(nMsg)
	{
	case SCN_SCAN_PROCESS_MESSAGE:
		{
			//GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]扫描进度:%.2f%%"), cDosDrive, ((float)lParam / (float)wParam) * 100 );
			break;
		}
	case SCN_LOAD_CACHE_PROCESS_MESSAGE:
		{
			//GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]加载缓存进度:%.2f%%"), cDosDrive, ((float)lParam / (float)wParam) * 100 );
			break;
		}
	case SCN_LOAD_CACHE_MESSAGE:
		{
			switch(wParam)
			{
			case LOAD_CACHE_MESSAGE_BEGIN:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]开始加载缓存文件"), cDosDrive);
				break;
			case LOAD_CACHE_MESSAGE_SUCCESS:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]加载缓存文件成功，耗时[%d]毫秒"), cDosDrive,lParam);
				break;
			case LOAD_CACHE_MESSAGE_FAILED:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]加载缓存文件失败"), cDosDrive);
				break;
			}
			break;
		}
	case SCN_SCAN_DISK_MESSAGE:
		{
			switch(wParam)
			{
			case SCAN_DISK_MESSAGE_BEGIN:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]开始扫描磁盘"), cDosDrive);
				break;
			case SCAN_DISK_MESSAGE_CANCEL:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]取消扫描磁盘"), cDosDrive);
				break;
			case SCAN_DISK_MESSAGE_SUCCESS:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]扫描磁盘成功，耗时[%d]毫秒"), cDosDrive, lParam);
				break;
			case SCAN_DISK_MESSAGE_FAILED:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]扫描磁盘失败"), cDosDrive);
				break;
			}
			break;
		}
	case SCN_SUCESS_MESSAGE:
		{
			UTIL::com_ptr<IDiskSearch> pDiskSearch;
			GetSearchDisk(cDosDrive, (IMSBase**)&pDiskSearch);
			if (pDiskSearch)
			{
				InitFileMark(cDosDrive,pDiskSearch);
			}

			GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]加载成功"), cDosDrive);
			break;
		}
	case SCN_FAILED_MESSAGE:
		{
			GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]加载失败"), cDosDrive);
			break;
		}
	case SCN_UNLOAD_MESSAGE:
		{
			switch(wParam)
			{
			case UNLOAD_MESSAGE_FAILED:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]卸载失败"), cDosDrive);
				break;
			case UNLOAD_MESSAGE_SUCCESS:
				GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]卸载成功，，耗时[%d]毫秒"), cDosDrive,lParam);
				break;
			}
		}
	}

	
	
	return S_OK;
}*/

STDMETHODIMP_(DWORD) CDiskSearchSrv::GetSearchDisk(IMSBase /*IProperty2*/** pSerchDisk)
{
	AUTOLOCK_CS(disk_map);
	UTIL::com_ptr<IProperty2>	pProp;
	m_pRot->CreateInstance(CLSID_CProperty2_thread, NULL, re_uuidof(IProperty2), (void**)&pProp);
	RASSERT(pProp, -1);
	CPropSet propSet(pProp);

	int nIndex = 0;
	for (CDiskMap::iterator it = m_diskMap.begin() ; it != m_diskMap.end() ; it++)
	{
		propSet[nIndex] = (DWORD)it->first;
		nIndex++;
	}

	pProp->QueryInterface(re_uuidof(IMSBase), (void**)pSerchDisk);
	return nIndex;
}


STDMETHODIMP CDiskSearchSrv::GetSearchDisk(WCHAR cDosName, IMSBase/*IDiskSearch*/** pDisk)
{
	if(cDosName>=L'a') cDosName-=32;//变大写

	UTIL::com_ptr<IDiskSearch> pDiskSearch;
	{
		AUTOLOCK_CS(disk_map);
		CDiskMap::iterator it = m_diskMap.find(cDosName);
		RTEST(it == m_diskMap.end(), E_FAIL);
		pDiskSearch = it->second;
	}

	RASSERT(pDiskSearch, E_FAIL);

	return pDiskSearch->QueryInterface(re_uuidof(IMSBase), (void**)pDisk);
}


HRESULT CDiskSearchSrv::CreateDiskSearch(WCHAR cDosName)
{
	AUTOLOCK_CS(disk_map);
	if(cDosName>=L'a') cDosName-=32;//变大写
	RASSERT(m_diskMap.find(cDosName) == m_diskMap.end(),S_OK);

	CLSID diskClsid = GUID_NULL;
	if (!GetDiskClsid(cDosName, &diskClsid) && diskClsid == GUID_NULL)
	{
		return E_FAIL;
	}


	UTIL::com_ptr<IDiskSearch> pDiskSearch = INULL;
	RFAILED(m_pRot->CreateInstance(diskClsid,NULL, re_uuidof(IDiskSearch) ,(void**)&pDiskSearch));
	RASSERT(pDiskSearch, E_FAIL);

	TCHAR strValue[MAX_PATH] = { 0 };
	m_bootValue.GetVaule(CHECK_INDEX_INVALUDE, 0, strValue, MAX_PATH);
	INT nCount = _ttoi(strValue);

	if (nCount > 3)
	{
		GrpError(GroupName,MsgLevel_Error, _T("读取索引缓存已超过错误上限[%d]"), nCount);
	}

	DWORD dwType = OpenDiskMask_UseCache;
	if (m_bReBuild || nCount > 3)
	{
		dwType |= OpenDiskMask_DeleteCacheFile;
	}

	HRESULT hr = pDiskSearch->OpenDisk(cDosName, this, dwType);
	if (FAILED(hr))
	{
		pDiskSearch->CloseDisk();
	}


	m_diskMap[cDosName] = pDiskSearch;

	return hr;
}


HRESULT CDiskSearchSrv::DeleteDiskSearch(WCHAR cDosName)
{

	{//删除状态
		{
			AUTOLOCK_CS(m_FileCacheMap);
			m_FileCacheMap.erase(cDosName);
		}
		{
			AUTOLOCK_CS(m_FileProgressCacheMap);
			m_FileProgressCacheMap.erase(cDosName);
		}

		{
			AUTOLOCK_CS(m_DiskStateCacheMap);
			m_DiskStateCacheMap.erase(cDosName);
		}
	}
	
	{//删除对象
		UTIL::com_ptr<IDiskSearch> pDisk;
		{
			AUTOLOCK_CS(disk_map);
			CDiskMap::iterator it = m_diskMap.find(cDosName);
			if (it != m_diskMap.end())
			{
				pDisk = it->second;
				m_diskMap.erase(it);
			}
		}
	

		if (pDisk)
		{
			pDisk->UnLoadDisk();
			pDisk->CloseDisk();
		}
	}

	return S_OK;
}

BOOL CDiskSearchSrv::GetDiskClsid(WCHAR cDosDriver, CLSID* clsid /*= NULL*/)
{
	if(cDosDriver>=L'a') cDosDriver-=32;//变大写
	TCHAR strDrivers[MAX_PATH] = {0};
	_stprintf_s(strDrivers,MAX_PATH,_T("%c:\\"),cDosDriver);
	UINT driveType = GetDriveType(strDrivers);

	RASSERT(!(DRIVE_FIXED != driveType && DRIVE_REMOVABLE != driveType) ,FALSE);

	DWORD dwMaxComLen = 0,dwFileSysFlag = 0;
	TCHAR szVolumeName[MAX_PATH] = {0};
	TCHAR fileSysBuf[MAX_PATH] = {0};
	RASSERT(GetVolumeInformation(strDrivers,szVolumeName,MAX_PATH,NULL,&dwMaxComLen,&dwFileSysFlag,fileSysBuf,MAX_PATH),FALSE);
	if (_tcsncmp(fileSysBuf, _T("NTFS"), 4) == 0)
	{
		if (clsid)
		{
			*clsid = CLSID_Ntfs;
		}

		return TRUE;
	}
	else
	{
		if (clsid)
		{
			*clsid = CLSID_Fat;
		}

		return TRUE;
	}
	return FALSE;
}


STDMETHODIMP CDiskSearchSrv::Lock()
{
	AUTOLOCK_CS(disk_map);
	for (CDiskMap::iterator it = m_diskMap.begin() ; it != m_diskMap.end() ; it++)
	{
		UTIL::com_ptr<IDiskSearch> pDisk = it->second;
		if (pDisk)
		{
			pDisk->LockDisk();
		}
	}
	return S_OK;
}


STDMETHODIMP CDiskSearchSrv::UnLock()
{
	AUTOLOCK_CS(disk_map);
	for (CDiskMap::iterator it = m_diskMap.begin() ; it != m_diskMap.end() ; it++)
	{
		UTIL::com_ptr<IDiskSearch> pDisk = it->second;
		if (pDisk)
		{
			pDisk->UnLockDisk();
		}
	}

	return S_OK;
}


WCHAR szSelFile[] = L"SELECT * FROM \"%s\" WHERE \"file\" = \"%s\"";
WCHAR szUpSql[]	= L"UPDATE \"%s\" SET leave=%d WHERE file=\"%s\"";

WCHAR szIns[]	= L"INSERT INTO \"%s\"(file,leave,version) VALUES(\"%s\", %d , 0)";
WCHAR szDel[]	= L"DELETE FROM \"%s\" WHERE file = \"%s\"";  
STDMETHODIMP CDiskSearchSrv::SetFileMark(LPCWSTR _lpszFile, DWORD dwLeave)
{
	AUTOLOCK_CS(disk_map);
	RASSERT(_lpszFile && wcslen(_lpszFile), E_INVALIDARG);
	WCHAR cDosName = _lpszFile[0];
	if(cDosName>=L'a') cDosName-=32;//变大写

	CString strFile = _lpszFile;
	strFile.MakeLower();
	LPCWSTR lpszFile = strFile.GetBuffer();


	do 
	{
		RASSERT2(m_pGenericDB, break);
		CString strSel; strSel.Format(szSelFile, m_strFileMarkTableName.GetBuffer(), lpszFile);

		UTIL::com_ptr<IGenericMS>	pRS;
		m_pGenericDB->Execute(strSel, &pRS.m_p);
		RASSERT2(pRS, break);

		CString strSql;
		if (pRS && !pRS->IsEOF())
		{
			if(dwLeave == 0)
			{
				strSql.Format(szDel, m_strFileMarkTableName.GetBuffer(), lpszFile);
				//删除记录
			}
			else
			{
				//更新记录
				strSql.Format(szUpSql, m_strFileMarkTableName.GetBuffer(), dwLeave, lpszFile);
			}

		}
		else if(dwLeave != 0)
		{
			strSql.Format(szIns, m_strFileMarkTableName.GetBuffer(), lpszFile, dwLeave);
		}
		if (strSql.GetLength())
		{
			if (FAILED(m_pGenericDB->ExecuteDML(strSql)))
			{
				GrpError(GroupName, MsgLevel_Msg, _T("CFileSearchSrv::SetFileMark::ExecuteDML(%s)"), strSql.GetBuffer());
			}
		}


	} while (FALSE);


	CDiskMap::iterator it = m_diskMap.find(cDosName);
	RASSERT(it != m_diskMap.end() , E_FAIL);

	UTIL::com_ptr<IDiskSearch> pDisk = it->second;
	return pDisk->SetFileMark(lpszFile, dwLeave);
}

STDMETHODIMP CDiskSearchSrv::ReBuildIndex()
{
	std::set<WCHAR> tempVolume;
	{
		AUTOLOCK_CS(disk_map);
		for (CDiskMap::iterator it = m_diskMap.begin() ; it != m_diskMap.end() ; it++)
		{
			tempVolume.insert(it->first);
		}
	}

	m_bReBuild = TRUE;
	for (std::set<WCHAR>::iterator it = tempVolume.begin() ; it != tempVolume.end() ; it++)
	{
		WCHAR cDosDriver = *it;
		TCHAR strDrivers[MAX_PATH] = {0};
		_stprintf_s(strDrivers,MAX_PATH,_T("%c:\\"),cDosDriver);
		OnDiskChange(DiskCtrl_uMount, DiskType_Unknown, strDrivers);
		OnDiskChange(DiskCtrl_Ready, DiskType_Unknown, strDrivers);
	}

	m_bReBuild = FALSE;
	return S_OK;
}

STDMETHODIMP_(DWORD) CDiskSearchSrv::GetState()
{
	AUTOLOCK_CS(m_DiskStateCacheMap);
	for ( CDiskStateCacheMap::iterator it = m_DiskStateCacheMap.begin() ; it != m_DiskStateCacheMap.end() ; it++)
	{
		//只上报完成和卸载的状态
		if ( it->second.dwState == DiskState_Scaning )
			return DiskState_Scaning;
	}
	return DiskState_Finish;
}

WCHAR szCreateTableSQL[] = 
	L"CREATE TABLE \"%s\" ("
	L"\"file\"  TEXT NOT NULL,"
	L"\"leave\"  INTEGER NOT NULL DEFAULT 0,"
	L"\"version\"  INTEGER,"
	L"\"reserve\"  TEXT"
	L")";

HRESULT CDiskSearchSrv::CreateSqliteDB()
{
	RASSERT(m_pRot, E_FAIL);

	m_pRot->CreateInstance(/*CLSID_DbSqliteSafeDB*/CLSID_DbSqliteDB, NULL, __uuidof(IGenericDB), (void**)&m_pGenericDB);
	RASSERT(m_pGenericDB, E_FAIL);

	UTIL::com_ptr<IMsEnv> pEnv;
	m_pRot->GetObject(CLSID_MsEnv, __uuidof(IMsEnv), (void**)&pEnv);
	RASSERT(pEnv, E_FAIL);

	UTIL::com_ptr<IPropertyStr> pProxyStr;
	pEnv->GetEnvParam((IMSBase**)&pProxyStr.m_p);
	RASSERT(pProxyStr, E_FAIL);

	CPropStrSet propSet(pProxyStr);
	LPCWSTR lpszproductname = propSet["productname"];

	if (!(lpszproductname && wcslen(lpszproductname)))
	{
		lpszproductname = L"FileSearchEng";
	}

	CString strPath;
	msapi::CApp(lpszproductname).GetDataPath(strPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strPath.ReleaseBuffer();
	strPath.Append(_T("\\filemark.sdb"));


	
	

	CString strConnect;strConnect.Format(_T("Database=%s;Pwd=password"), strPath.GetBuffer());
	RFAILED(m_pGenericDB->Connect(/*strConnect*/strPath));

	LPCWSTR lpszNameSpace = propSet["prouctid"];
	RASSERT(lpszNameSpace && wcslen(lpszNameSpace), E_FAIL);

	m_strFileMarkTableName = lpszNameSpace;


	if (!m_pGenericDB->IsTableExists(m_strFileMarkTableName))
	{
		CString strCreate; strCreate.Format(szCreateTableSQL, m_strFileMarkTableName.GetBuffer());
		if (FAILED(m_pGenericDB->ExecuteDML(strCreate)))
		{
			GrpError(GroupName, MsgLevel_Error, _T("CFileSearchSrv::CreateSqliteDB::ExecuteDML(%s)"), strCreate.GetBuffer());
			return E_FAIL;
		}
	}

	return S_OK;

}

HRESULT CDiskSearchSrv::DestorySqliteDB()
{
	if (m_pGenericDB)
	{
		m_pGenericDB->Close();
		m_pGenericDB = INULL;
	}

	return S_OK;
}

WCHAR szSelSql[] = L"SELECT * FROM \"%s\" ";

HRESULT CDiskSearchSrv::InitFileMark(WCHAR wDos, IDiskSearch* pDisk)
{
	RASSERT(m_pGenericDB && pDisk, E_FAIL);

	WCHAR cDosName = wDos;
	if(cDosName>=L'a') cDosName-=32;//变大写

	CString strSel; strSel.Format(szSelSql, m_strFileMarkTableName.GetBuffer());

	UTIL::com_ptr<IGenericMS>	pRS;
	if (FAILED(m_pGenericDB->Execute(strSel, &pRS.m_p)))
	{
		GrpError(GroupName, MsgLevel_Error, _T("CFileSearchSrv::InitFileMark::Execute(%s)"), strSel.GetBuffer());
		return E_FAIL;
	}

	while( !pRS->IsEOF() )
	{
		UTIL::com_ptr<IPropertyStr>	prop;
		if (SUCCEEDED(pRS->GetRecord( &prop.m_p )))
		{
			CPropStrSet propStrSet( prop );
			LPTSTR	 szFile = propStrSet["file"];
			int		 nLeave = propStrSet["leave"];
			int		 nVer = propStrSet["version"];
			LPTSTR	 szReserve = propStrSet["reserve"];

			WCHAR cDos = szFile[0];
			if(cDos>=L'a') cDos-=32;//变大写

			if(cDos == cDosName)
			{
				if (FAILED(pDisk->SetFileMark(szFile, nLeave)))
				{
					GrpError(GroupName, MsgLevel_Error, _T("InitFileMark::SetFileMark(%s,%d)"), szFile, nLeave);
				}
			}
		}

		if(FAILED(pRS->MoveNext()))
		{
			break;
		}
	}

	return S_OK;
}