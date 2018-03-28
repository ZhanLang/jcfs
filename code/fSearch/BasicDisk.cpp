#include "StdAfx.h"
#include "BasicDisk.h"

#include "stdafx.h"
#include "combase\IMsVector.h"
#include "fSearchDef.h"

enum 
{
	WorkStep_Scan = 100,
	WorkStep_Load,
	WorkStep_Unload,
};

CBaseDevice::CBaseDevice():m_Worker(this)
{
	ZeroMemory(&m_Mpool, sizeof(m_Mpool));
	mpool_init_default(&m_Mpool, 4196, 1024);

	ZeroMemory(m_strDataPath, sizeof(m_strDataPath));
	m_isScanFinesh = 0;
	m_bLockDisk = FALSE;

	m_OpenDiskMask = OpenDiskMask_UseCache;
	m_dwCurState = DiskState_UnKnown;

	m_bCancle = FALSE;
	m_dwReNameOldFileReferenceNumber = 0;
	m_bWriteDb = FALSE;
}

CBaseDevice::~CBaseDevice()
{
	mpool_destroy(&m_Mpool);
}



STDMETHODIMP CBaseDevice::OpenDisk(WCHAR cDosName, IDiskSearchNotify* pNotify, DWORD/*OpenDiskMask*/ openMask)
{
	RASSERT(pNotify, E_INVALIDARG);

	m_bWriteDb = FALSE;
	m_pDiskSearchNotify = pNotify;
	m_OpenDiskMask = openMask;
	m_cDosDriver = UpCase(cDosName);
	
	m_dwVolumeSerialNumber = GetVolumeVolumeSerialNumber(m_cDosDriver);
	
	//和文件监视服务建立连接点
	m_pRot->GetObject(CLSID_FileMonitorSrv, re_uuidof(IMSBase), (void**)&m_pFileMonitorService.m_p);
	RASSERT(m_pFileMonitorService, E_FAIL);
	RFAILEDP(m_UseFileMonitorConnectPoint.Connect(m_pFileMonitorService, (IUnknown*)((IDiskSearch*)(this))), FALSE);



	WCHAR szVolumePath[8];
	swprintf_s(szVolumePath, _countof(szVolumePath), L"%c:\\", m_cDosDriver);


	DWORD lpVolumeSerialNumber = 0;
	RASSERT(GetVolumeInformation(szVolumePath, NULL, 0, &lpVolumeSerialNumber, NULL, NULL, NULL, NULL), E_FAIL);

	RASSERT(m_extenNameMgr.SetNameSpace(GetEnvParamString("productname"),m_dwVolumeSerialNumber), E_FAIL);

	return S_OK;
}

STDMETHODIMP CBaseDevice::LoadDisk()
{
	m_bWriteDb = FALSE;
	if( m_OpenDiskMask == OpenDiskMask_UseCache )
		m_Worker.Dispatch(WorkStep_Load);
	else
		m_Worker.Dispatch(WorkStep_Scan);

	return S_OK;
}

STDMETHODIMP CBaseDevice::UnLoadDisk()
{

	m_isScanFinesh = FALSE;
	//m_Worker.WaitForExit();//首先得等待扫描完成
	if ( m_dwCurState == DiskState_Finish)
		m_Worker.Dispatch(WorkStep_Unload);

	m_Worker.WaitForExit();
	return S_OK;
}

STDMETHODIMP CBaseDevice::CloseDisk()
{
	m_isScanFinesh = FALSE;
	m_UseFileMonitorConnectPoint.DisConnect();

	//UnLoadDisk();
	//等待所有的任务都退出
	

	return S_OK;
}




#define QueryDecodeNameLen 512
struct QueryDecode
{
	WCHAR strSearchName[QueryDecodeNameLen];
	BYTE pNameInformation[QueryDecodeNameLen];
	INT  nDecodeNameLen;
};


//查询,返回记录个数
STDMETHODIMP CBaseDevice::Query(const QueryCondition& Condition, IMsDwordVector* pDwordVector)
{
	CAutoReadLock readLock(m_rwQueryLock);
	RASSERT(pDwordVector, E_INVALIDARG);

	if (!m_isScanFinesh)
		return 0;
	
	DWORD m_dwFileResultCount = 0;
	DWORD m_dwDirResultCount = 0;
	DWORD dwTickCount = GetTickCount();

	RASSERT(Condition.lpstrFileName, 0); //文件名为空

	//搜索条件不匹配
	if (!(Condition.dwConditionMask & QueryConditionMask_File ||
		Condition.dwConditionMask & QueryConditionMask_Dir))
	{
		return 0;
	}



	CApiMap<INT, INT> ExtensionIdMap;
	//如果扩展名不为空，但是扩展名又不再扩展名列表中，直接返回吧
	if (!GetExtIdFromStr(ExtensionIdMap, Condition.lpstrExtension , Condition.dwConditionMask & QueryConditionMask_FullExt))
	{
		return 0;
	}



	BOOL bFiltleExtension = ExtensionIdMap.Count();

	//如果有扩展名，只搜文件类型
	BOOL bSearchDir = (Condition.dwConditionMask & QueryConditionMask_Dir) && (ExtensionIdMap.Count() == 0);
	BOOL bSearchFile = (Condition.dwConditionMask & QueryConditionMask_File);
	BOOL bCase = Condition.dwConditionMask & QueryConditionMask_Case;
	BOOL bFullWord = Condition.dwConditionMask & QueryConditionMask_Full && inline_wcslen(Condition.lpstrFileName);
	BOOL bHasSub = Condition.dwConditionMask & QueryConditionMask_Sub;

	BOOL bExclude_System = Condition.dwConditionMask & QueryConditionMask_Exclude_System;
	BOOL bExclude_Hidden = Condition.dwConditionMask & QueryConditionMask_Exclude_Hidden;

	//CloseMonitor();  //关闭实时更新

	//开始搜索
	WCHAR szSearchStr[1024] = { 0 };//当前要查询的串
	inline_wcscpy(szSearchStr, Condition.lpstrFileName);
	if (!bCase)
	{
		inline_wcswr(szSearchStr, inline_wcslen(szSearchStr));
	}

	QueryDecode queryDecode[MAX_PATH] = {0};
	INT nQueryDecodeCount = 0;

	BOOL isNONASCII = FALSE;


	//////////////////////////////////////////////////////////////////////////
	//加入特殊处理

	//@@  1   //‘:’的特殊处理,,查找根盘符
	if (szSearchStr[0] == ':' && szSearchStr[1] == 0)
	{
		_stprintf_s(szSearchStr, 1024, _T("%c:"), m_cDosDriver);
		bSearchDir = TRUE;
		bSearchFile = FALSE;
		bCase = TRUE;
		bFullWord = TRUE;
		bHasSub = FALSE;
	}


	//对需要查询的文件名称编码
	if (bFullWord)
	{
		PWCHAR pcc = szSearchStr;
		wcscpy(queryDecode[nQueryDecodeCount].strSearchName, szSearchStr);
		int nLength = _tcslen(szSearchStr);
		queryDecode[0].nDecodeNameLen = EncodFileName(pcc , nLength, (LPBYTE)(queryDecode[0].pNameInformation));
		queryDecode[0].pNameInformation[queryDecode[0].nDecodeNameLen] = 0;
		nQueryDecodeCount = 1;
		isNONASCII =  queryDecode[0].nDecodeNameLen > nLength;
	}
	else
	{
		TCHAR *next_token = NULL;
		TCHAR *szCmd = _tcstok_s(szSearchStr, L"| *", &next_token);
		while( szCmd != NULL && _tcslen(szCmd))
		{
			int nLength = _tcslen(szCmd);

			QueryDecode q = {0};

			wcscpy(q.strSearchName, szCmd);
			q.nDecodeNameLen = EncodFileName(szCmd , nLength, (LPBYTE)(q.pNameInformation));

			q.pNameInformation[q.nDecodeNameLen] = 0;
			isNONASCII |=  q.nDecodeNameLen > nLength;

			//插入排序
			if (nQueryDecodeCount)
			{
				//只要保证queryDecode[0]为最大即可
				if (queryDecode[0].nDecodeNameLen < q.nDecodeNameLen)
				{
					QueryDecode temp;
					memcpy(&temp, &queryDecode[0], sizeof(QueryDecode));
					memcpy(&queryDecode[0], &q, sizeof(QueryDecode));
					memcpy(&queryDecode[nQueryDecodeCount], &temp, sizeof(QueryDecode));
				}
				else
				{
					memcpy(&queryDecode[nQueryDecodeCount], &q, sizeof(QueryDecode));
				}
			}
			else
			{
				memcpy(&queryDecode[0], &q, sizeof(QueryDecode));
			}

			nQueryDecodeCount++;
			szCmd = _tcstok_s( NULL, L"| *", &next_token );
		}
	}


	
	CApiMap<DWORD,DWORD> IncludePathFilterMap;

	BOOL bSearcThisDriver = GetIncludePathFromStr(IncludePathFilterMap, Condition.lpstrIncludeDir);
	BOOL bFilterPath = IncludePathFilterMap.Count();
	if (bSearcThisDriver == FALSE)
	{
		//说明这里直接过滤掉了本盘符
		return 0;
	}

		
	if (!(bSearchFile || bSearchDir))
	{
		//OpenMonitor();
		return 0;
	}

	//搜索文件
	

	
	
	
	
	BOOL bCanSearch = TRUE;

	if (bSearchFile)  
	{
		WCHAR szFileName[MAX_PATH] = { 0 };
		DWORD dwFileNameLen = 0;
		DWORD dwFileCount = m_FilePoolIndex.Size();
		PFILE_RECORD_INFO pFile = NULL;

		PBYTE pNameInformation = NULL;
		INT   nNameInformationLen = 0;

		PFILE_RECORD_INFO* pPoint = m_FilePoolIndex.GetPoint();
		PFILE_RECORD_INFO pDir = NULL;
		BOOL bSearch = FALSE;
		BOOL bCompareNONASCII = FALSE;

		for (DWORD dwIndex = 0; dwIndex < dwFileCount; ++dwIndex)
		{
			bCanSearch = TRUE;
			pFile = pPoint[dwIndex];
			pDir = pFile->pParentDirRecord;
			bCompareNONASCII = FALSE;

			/*
			if (bExclude_Hidden && (pFile->Mark & BITMASK_FILE_ATTRIBUTE_HIDDEN))
			{
				continue;
			}
			*/
			/*
			if (bExclude_System && (pFile->Mark & BITMASK_FILE_ATTRIBUTE_SYSTEM))
			{
				continue;
			}
			*/

			if (pFile->BasicInfo.TwoByteNameLen)
			{
				pNameInformation = pFile->NameInformation + 2; 
				nNameInformationLen = *(WORD*)(pFile->NameInformation); 
			}
			else
			{
				pNameInformation = pFile->NameInformation + 1; 
				nNameInformationLen = *(pFile->NameInformation); 
			}

			bCompareNONASCII = (pFile->BasicInfo.NoNascii) != 0;
			//如果需要查询的字符串中有中文，但是在文件中没有，直接跳过
			if (isNONASCII && !bCompareNONASCII)
			{
				continue;
			}


			//如果文件名长度小于需要查找的字符串直接返回
			if (queryDecode[0].nDecodeNameLen > nNameInformationLen)
			{
				continue;
			}
			
			
		
			//需要进行文件夹过滤
			if (bFilterPath)
			{
				if(!pDir) 
					continue;;
				bSearch = FALSE;
				if (!bHasSub)
				{
					if (!IncludePathFilterMap.Find(pDir->BasicInfo.Index))
					{
						continue;
					}
				}
				else
				{
					do 
					{
						//只要其祖上是需要包含的就行
						if (IncludePathFilterMap.Find(pDir->BasicInfo.Index))
						{
							bSearch = TRUE;
							break;
						}
					} while (pDir = pDir->pParentDirRecord);
					
					if (!bSearch)
					{
						continue;
					}
				}
			}

			

			//需要进行扩展名过滤
			if (bFiltleExtension)
			{
				INT nExtenId = GetExtendID(pFile);
				if (nExtenId == 0)
				{
					continue;
				}

				if (!ExtensionIdMap.Find(nExtenId))
				{
					continue;
				}
			}


			if (nQueryDecodeCount == 0)
			{
				DWORD dwLeave = pFile->BasicInfo.Leave;
				if (dwLeave == 0)
				{
					pDwordVector->Add(GetResult(m_cDosDriver,TRUE, pFile->BasicInfo.Leave, pFile->BasicInfo.Index));
					m_dwFileResultCount++;
					continue;
				}

				pDwordVector->Lock();
				DWORD dwLoop = 0;
				for (; dwLoop < pDwordVector->GetCount() ; dwLoop++)
				{
					DWORD dwItemIndex = pDwordVector->GetAt(dwLoop);
					DWORD dwIndexLeave = GetResultLeave(dwItemIndex);

					if (dwIndexLeave < dwLeave)
					{
						break;
					}

					if (dwIndexLeave == dwLeave)
					{
						//暂时按盘符排序
						if (m_cDosDriver < GetResultVolume(dwItemIndex))
						{
							break;
						}
					}
				}


				pDwordVector->AddAt(dwLoop, GetResult(m_cDosDriver,TRUE, dwLeave, pFile->BasicInfo.Index));
				m_dwDirResultCount++;
				pDwordVector->UnLock();
				continue;
			}

			//如果需要搜索中文，但又不是匹配大小写的
			if (bCompareNONASCII)
			{
				if(!bCase)
				{
					DecodeFileNameNoCase(pNameInformation, nNameInformationLen, szFileName ); 
				}
				else
				{
					DecodeFileName(pNameInformation, nNameInformationLen, szFileName); 
				}
			}


			int nRet = 0;
			
			if (!bFullWord)
			{
				for (INT nIndex = 0; nIndex < nQueryDecodeCount; ++nIndex)
				{
					if (bCompareNONASCII)
					{
						WCHAR_WCSSTR(szFileName, queryDecode[nIndex].strSearchName,nRet);
					}
					else
					{
						if (bCase)
						{
							BYTE_WCSSTR(pNameInformation, nNameInformationLen, queryDecode[nIndex].pNameInformation,queryDecode[nIndex].nDecodeNameLen, nRet);
						}
						else
						{
							BYTE_WCSSTR_NOCASE(pNameInformation, nNameInformationLen, queryDecode[nIndex].pNameInformation,queryDecode[nIndex].nDecodeNameLen, nRet);
						}
					}
					if (!nRet)
					{
						break;
					}
				}
				if (nRet)
				{
					DWORD dwLeave = pFile->BasicInfo.Leave;
					if (dwLeave == 0)
					{
						pDwordVector->Add(GetResult(m_cDosDriver,TRUE, pFile->BasicInfo.Leave, pFile->BasicInfo.Index));
						m_dwFileResultCount++;
						continue;
					}

					pDwordVector->Lock();
					DWORD dwLoop = 0;
					for (; dwLoop < pDwordVector->GetCount() ; dwLoop++)
					{
						DWORD dwItemIndex = pDwordVector->GetAt(dwLoop);
						DWORD dwIndexLeave = GetResultLeave(dwItemIndex);

						if (dwIndexLeave < dwLeave)
						{
							break;
						}

						if (dwIndexLeave == dwLeave)
						{
							//暂时按盘符排序
							if (m_cDosDriver < GetResultVolume(dwItemIndex))
							{
								break;
							}
						}
					}


					pDwordVector->AddAt(dwLoop, GetResult(m_cDosDriver,TRUE, dwLeave, pFile->BasicInfo.Index));
					m_dwDirResultCount++;
					pDwordVector->UnLock();
				}
			}
			else
			{
				if (queryDecode[0].nDecodeNameLen != nNameInformationLen)
				{
					continue;
				}

				if (bCompareNONASCII)
				{
					WCHAR_WCSCMP(szFileName, queryDecode[0].strSearchName,nRet);
				}
				else
				{
					if (bCase)
					{
						BYTE_WCSCMP(pNameInformation, nNameInformationLen, (PBYTE)queryDecode[0].pNameInformation,queryDecode[0].nDecodeNameLen, nRet);
					}
					else
					{
						BYTE_WCSCMP_NO_CASE(pNameInformation, nNameInformationLen, queryDecode[0].pNameInformation,queryDecode[0].nDecodeNameLen, nRet);
					}
				}
				if (nRet)
				{
					DWORD dwLeave = pFile->BasicInfo.Leave;
					if (dwLeave == 0)
					{
						pDwordVector->Add(GetResult(m_cDosDriver,TRUE, pFile->BasicInfo.Leave, pFile->BasicInfo.Index));
						m_dwFileResultCount++;
						continue;
					}

					pDwordVector->Lock();
					DWORD dwLoop = 0;
					for (; dwLoop < pDwordVector->GetCount() ; dwLoop++)
					{
						DWORD dwItemIndex = pDwordVector->GetAt(dwLoop);
						DWORD dwIndexLeave = GetResultLeave(dwItemIndex);

						if (dwIndexLeave < dwLeave)
						{
							break;
						}

						if (dwIndexLeave == dwLeave)
						{
							//暂时按盘符排序
							if (m_cDosDriver < GetResultVolume(dwItemIndex))
							{
								break;
							}
						}
					}


					pDwordVector->AddAt(dwLoop, GetResult(m_cDosDriver,TRUE, dwLeave, pFile->BasicInfo.Index));
					m_dwDirResultCount++;
					pDwordVector->UnLock();
				}
			}
		}
	}
	
	if (bSearchDir) 
	{
		WCHAR szFileName[MAX_PATH] = { 0 };
		DWORD dwFileNameLen = 0;
		DWORD dwFileCount = m_DirPoolIndex.Size();
		PFILE_RECORD_INFO pFile = NULL;

		PBYTE pNameInformation = NULL;
		INT   nNameInformationLen = 0;

		PFILE_RECORD_INFO* pPoint = m_DirPoolIndex.GetPoint();
		PFILE_RECORD_INFO pDir = NULL;
		BOOL bSearch = FALSE;
		BOOL bCompareNONASCII = FALSE;

		for (DWORD dwIndex = 0; dwIndex < dwFileCount; ++dwIndex)
		{
			bCanSearch = TRUE;
			pFile = pPoint[dwIndex];
			pDir = pFile->pParentDirRecord;
			bCompareNONASCII = FALSE;

			/*
			if (bExclude_Hidden && (pFile->Mark & BITMASK_FILE_ATTRIBUTE_HIDDEN))
			{
				continue;
			}
			*/
			/*
			if (bExclude_System && (pFile->Mark & BITMASK_FILE_ATTRIBUTE_SYSTEM))
			{
				continue;
			}
			*/

			if (pFile->BasicInfo.TwoByteNameLen)
			{
				pNameInformation = pFile->NameInformation + 2; 
				nNameInformationLen = *(WORD*)(pFile->NameInformation); 
			}
			else
			{
				pNameInformation = pFile->NameInformation + 1; 
				nNameInformationLen = *(pFile->NameInformation); 
			}

			bCompareNONASCII = (pFile->BasicInfo.NoNascii) != 0;
			//如果需要查询的字符串中有中文，但是在文件中没有，直接跳过
			if (isNONASCII && !bCompareNONASCII)
			{
				continue;
			}


			//如果文件名长度小于需要查找的字符串直接返回
			if (queryDecode[0].nDecodeNameLen > nNameInformationLen)
			{
				continue;
			}
			
			
		
			//需要进行文件夹过滤
			if (bFilterPath)
			{
				if(!pDir) 
					continue;;
				bSearch = FALSE;
				if (!bHasSub)
				{
					if (!IncludePathFilterMap.Find(pDir->BasicInfo.Index))
					{
						continue;
					}
				}
				else
				{
					do 
					{
						//只要其祖上是需要包含的就行
						if (IncludePathFilterMap.Find(pDir->BasicInfo.Index))
						{
							bSearch = TRUE;
							break;
						}
					} while (pDir = pDir->pParentDirRecord);
					
					if (!bSearch)
					{
						continue;
					}
				}
			}

			

			//需要进行扩展名过滤
			if (bFiltleExtension)
			{
				INT nExtenId = GetExtendID(pFile);
				if (nExtenId == 0)
				{
					continue;
				}

				if (!ExtensionIdMap.Find(nExtenId))
				{
					continue;
				}
			}


			if (nQueryDecodeCount == 0)
			{

				
				DWORD dwLeave = pFile->BasicInfo.Leave;
				if (dwLeave == 0)
				{
					pDwordVector->Add(GetResult(m_cDosDriver,FALSE, pFile->BasicInfo.Leave, pFile->BasicInfo.Index));
					m_dwFileResultCount++;
					continue;
				}

				pDwordVector->Lock();
				DWORD dwLoop = 0;
				for (; dwLoop < pDwordVector->GetCount() ; dwLoop++)
				{
					DWORD dwItemIndex = pDwordVector->GetAt(dwLoop);
					DWORD dwIndexLeave = GetResultLeave(dwItemIndex);

					if (dwIndexLeave < dwLeave)
					{
						break;
					}

					if (dwIndexLeave == dwLeave)
					{
						//暂时按盘符排序
						if (m_cDosDriver < GetResultVolume(dwItemIndex))
						{
							break;
						}
					}
				}


				pDwordVector->AddAt(dwLoop, GetResult(m_cDosDriver,FALSE, dwLeave, pFile->BasicInfo.Index));
				m_dwDirResultCount++;
				pDwordVector->UnLock();
				continue;
			}

			//如果需要搜索中文，但又不是匹配大小写的
			if (bCompareNONASCII)
			{
				if(!bCase)
				{
					DecodeFileNameNoCase(pNameInformation, nNameInformationLen, szFileName ); 
				}
				else
				{
					DecodeFileName(pNameInformation, nNameInformationLen, szFileName); 
				}
			}


			int nRet = 0;
			
			if (!bFullWord)
			{
				for (INT nIndex = 0; nIndex < nQueryDecodeCount; ++nIndex)
				{
					if (bCompareNONASCII)
					{
						WCHAR_WCSSTR(szFileName, queryDecode[nIndex].strSearchName,nRet);
					}
					else
					{
						if (bCase)
						{
							BYTE_WCSSTR(pNameInformation, nNameInformationLen, queryDecode[nIndex].pNameInformation,queryDecode[nIndex].nDecodeNameLen, nRet);
						}
						else
						{
							BYTE_WCSSTR_NOCASE(pNameInformation, nNameInformationLen, queryDecode[nIndex].pNameInformation,queryDecode[nIndex].nDecodeNameLen, nRet);
						}
					}
					if (!nRet)
					{
						break;
					}
				}
				if (nRet)
				{
					DWORD dwLeave = pFile->BasicInfo.Leave;
					if (dwLeave == 0)
					{
						pDwordVector->Add(GetResult(m_cDosDriver,FALSE, pFile->BasicInfo.Leave, pFile->BasicInfo.Index));
						m_dwFileResultCount++;
						continue;
					}

					pDwordVector->Lock();
					DWORD dwLoop = 0;
					for (; dwLoop < pDwordVector->GetCount() ; dwLoop++)
					{
						DWORD dwItemIndex = pDwordVector->GetAt(dwLoop);
						DWORD dwIndexLeave = GetResultLeave(dwItemIndex);

						if (dwIndexLeave < dwLeave)
						{
							break;
						}

						if (dwIndexLeave == dwLeave)
						{
							//暂时按盘符排序
							if (m_cDosDriver < GetResultVolume(dwItemIndex))
							{
								break;
							}
						}
					}


					pDwordVector->AddAt(dwLoop, GetResult(m_cDosDriver,FALSE, dwLeave, pFile->BasicInfo.Index));
					m_dwDirResultCount++;
					pDwordVector->UnLock();
				}
			}
			else
			{
				if (queryDecode[0].nDecodeNameLen != nNameInformationLen)
				{
					continue;
				}

				if (bCompareNONASCII)
				{
					WCHAR_WCSCMP(szFileName, queryDecode[0].strSearchName,nRet);
				}
				else
				{
					if (bCase)
					{
						BYTE_WCSCMP(pNameInformation, nNameInformationLen, (PBYTE)queryDecode[0].pNameInformation,queryDecode[0].nDecodeNameLen, nRet);
					}
					else
					{
						BYTE_WCSCMP_NO_CASE(pNameInformation, nNameInformationLen, queryDecode[0].pNameInformation,queryDecode[0].nDecodeNameLen, nRet);
					}
				}
				if (nRet)
				{
					DWORD dwLeave = pFile->BasicInfo.Leave;
					if (dwLeave == 0)
					{
						pDwordVector->Add(GetResult(m_cDosDriver,FALSE, pFile->BasicInfo.Leave, pFile->BasicInfo.Index));
						m_dwFileResultCount++;
						continue;
					}

					pDwordVector->Lock();
					DWORD dwLoop = 0;
					for (; dwLoop < pDwordVector->GetCount() ; dwLoop++)
					{
						DWORD dwItemIndex = pDwordVector->GetAt(dwLoop);
						DWORD dwIndexLeave = GetResultLeave(dwItemIndex);

						if (dwIndexLeave < dwLeave)
						{
							break;
						}

						if (dwIndexLeave == dwLeave)
						{
							//暂时按盘符排序
							if (m_cDosDriver < GetResultVolume(dwItemIndex))
							{
								break;
							}
						}
					}


					pDwordVector->AddAt(dwLoop, GetResult(m_cDosDriver,FALSE, dwLeave, pFile->BasicInfo.Index));
					m_dwDirResultCount++;
					pDwordVector->UnLock();
				}
			}
		}
	}

	dwTickCount = GetTickCount() - dwTickCount;
	GrpMsg(GroupName, MsgLevel_Msg, _T("盘符[%c]检索到了%d个文件，耗时%d"), m_cDosDriver, m_dwDirResultCount + m_dwFileResultCount, dwTickCount);
	return S_OK;
}




STDMETHODIMP CBaseDevice::OnFileChangeNotify(DWORD dwMask, LPCWSTR lpName, DWORD dwAttribute)
{
	return S_OK;
}

STDMETHODIMP CBaseDevice::OnFileChangeTimeOut(DWORD dwTimer)
{
	if ( m_isScanFinesh )
	{
		CAutoWriteLock readLock(m_rwQueryLock);
		LoadDiskLog();
		m_pDiskSearchNotify->OnDiskSearch_FileCountChange(m_cDosDriver, m_FilePoolIndex.Size(), m_DirPoolIndex.Size());	
	}
	
	return S_OK;
}

STDMETHODIMP CBaseDevice::OnAppendMonitor(LPCWSTR lpVolumes, BOOL succeed)
{
	return S_OK;
}

STDMETHODIMP CBaseDevice::OnRemoveMonitor(LPCWSTR szVolumes, BOOL succeed)
{
	return S_OK;
}

LPCTSTR CBaseDevice::GetDataPath()
{
	if ( !_tcslen( m_strDataPath ) )
	{
		msapi::CApp(GetEnvParamString("productname")).GetDataPath(m_strDataPath, _countof(m_strDataPath));
		_tcscat_s( m_strDataPath, _countof(m_strDataPath), _T("fscache\\"));
		msapi::CreateDirectoryEx(m_strDataPath);
	}

	return m_strDataPath;
}

//获取文件个数
DWORDLONG CBaseDevice::GetCount(DWORD* pdwFileCount, DWORD* pdwDirCount)
{
	if ( pdwDirCount )
		*pdwDirCount = m_DirPoolIndex.Size();
	
	if ( pdwFileCount )
		*pdwFileCount = m_FilePoolIndex.Size();
	
	return m_DirPoolIndex.Size() + m_FilePoolIndex.Size();
}



STDMETHODIMP_(WCHAR) CBaseDevice::GetVolumeName()
{
	return m_cDosDriver;
}

STDMETHODIMP_(DWORD) CBaseDevice::GetCurrentState()
{
	return m_dwCurState;
}

STDMETHODIMP CBaseDevice::LockDisk()
{
	m_bLockDisk = TRUE;
	
	return S_OK;
}

STDMETHODIMP CBaseDevice::UnLockDisk()
{
	m_bLockDisk = FALSE;
	//OnOpenMonitor();
	return S_OK;
}

STDMETHODIMP_(BOOL) CBaseDevice::IsLockDisk()
{
	return FALSE;
}

STDMETHODIMP CBaseDevice::GetResultAt( DWORD dwResultIndex, DWORD GetValueMask,SearchResult& info)
{
	WCHAR cDosName = GetResultVolume(dwResultIndex);
	RASSERT(cDosName == m_cDosDriver, E_FAIL);

	BOOL bFile	  = IsResultFile(dwResultIndex);

	TCHAR szName[MAX_PATH] = { 0 };
	DWORD dwPathLen = 0;
	CAutoReadLock readLock(m_rwQueryLock);


	if (bFile)//查询文件
	{
		DWORD dwIndex = FindFile(GetResultIndex(dwResultIndex));
		RTEST(dwIndex == -1, E_FAIL);

		PFILE_RECORD_INFO pFile = m_FilePoolIndex[dwIndex];
		RASSERT(pFile, E_FAIL);

		//填充级别
		info.dwLeave = pFile->BasicInfo.Leave;

		//填充文件名
		GetFileName(pFile, info.szName);

		//填充路径
		GetFilePath(pFile, info.szPath);

		//填充扩展名
		DWORD nExtenID = GetExtendID(pFile);
		if ( nExtenID )
		{
			LPCWSTR lpszExtenName = m_extenNameMgr.GetExtenName(nExtenID);
			inline_wcscat(info.szExtension, lpszExtenName);
			inline_wcscat(info.szName, L".");
			inline_wcscat(info.szName, info.szExtension);

			if ( GetValueMask & GetValueMask_IcoIndex )
			{
				INT nIconID = m_extenNameMgr.GetIconId(nExtenID);

				if (nIconID == ICON_INDEX_NOICON || nIconID == ICON_INDEX_REALTIMECALL || nIconID == ICON_INDEX_UNINITIALIZED)
				{
					TCHAR szFullPath[MAX_PATH] = {0};
					inline_wcscat(szFullPath,info.szPath);
					inline_wcscat(szFullPath,info.szName);
					INT nIcon = CExtenNameMgr::GetFileIconIndex(szFullPath);

					if (nIconID != ICON_INDEX_REALTIMECALL || nIconID == ICON_INDEX_UNINITIALIZED)
					{
						m_extenNameMgr.SetIconId(nExtenID,nIcon);
					}
					nIconID = nIcon;
				}
				info.dwIconIndex = nIconID;
			}
		}
	}
	else
	{
		DWORD dwIndex = FindDirectory(GetResultIndex(dwResultIndex));
		RTEST(dwIndex == -1, E_FAIL);

		PFILE_RECORD_INFO pFile = m_DirPoolIndex[dwIndex];
		RASSERT(pFile, E_FAIL);

		//填充级别
		info.dwLeave = pFile->BasicInfo.Leave;

		//填充文件名
		GetFileName(pFile, info.szName);

		//填充路径
		GetFilePath(pFile, info.szPath);


		if ( GetValueMask & GetValueMask_IcoIndex )
		{
			TCHAR szFullPath[MAX_PATH] = { 0 };
			DWORD dwNameLen = 0;
			if (dwNameLen == 0)
			{
				swprintf_s(szFullPath, _countof(szFullPath), L"%s\\*.*", info.szName);
			}
			else
			{
				swprintf_s(szFullPath, _countof(szFullPath), L"%s\\%s", info.szPath, info.szName);
			}

			if (dwNameLen == 0)  //盘符
			{
				if (info.szName[0] == 'C')
				{
					info.dwIconIndex = CExtenNameMgr::GetSysRootIcon();
				}
				else
				{
					info.dwIconIndex = CExtenNameMgr::GetRootIcon();
				}
				if (info.dwIconIndex == -1)
				{
					CString strFilePath;
					strFilePath.Format(_T("%s\\"),info.szName);
					SHFILEINFO FileInfo = {0};
					::SHGetFileInfo(strFilePath,0, &FileInfo, sizeof(SHFILEINFO),	SHGFI_SYSICONINDEX);
					info.dwIconIndex = FileInfo.iIcon;
				}
			}
			else
			{
				info.dwIconIndex = CExtenNameMgr::GetDirectoryIcon();
			}
		}
		
	}

	TCHAR szFullPath[ MAX_PATH ] = { 0 };
	if ( _tcslen( info.szPath) == 0)
	{
		//_tcscat_s( szFullPath, MAX_PATH, info.szName);
		//_tcscat_s( szFullPath, MAX_PATH, _T("\\"));
		swprintf_s(szFullPath, _countof(szFullPath), L"%s\\*.*", info.szName);
	}
	else
	{
		_tcscpy_s( szFullPath, MAX_PATH, info.szPath);
		_tcscat_s( szFullPath, MAX_PATH, info.szName);
	}

	WIN32_FIND_DATA fileData = {0};
	HANDLE hFind = FindFirstFile(szFullPath , &fileData);
	if ( hFind == INVALID_HANDLE_VALUE) return 0;
	FindClose(hFind);
	LARGE_INTEGER llSize = { 0 };
	llSize.LowPart = fileData.nFileSizeLow;
	llSize.HighPart = fileData.nFileSizeHigh;


	LARGE_INTEGER llAccessTime= { 0 };
	llAccessTime.LowPart = fileData.ftLastAccessTime.dwLowDateTime;
	llAccessTime.HighPart = fileData.ftLastAccessTime.dwHighDateTime;

	LARGE_INTEGER llCreateTime= { 0 };
	llCreateTime.LowPart = fileData.ftCreationTime.dwLowDateTime;
	llCreateTime.HighPart = fileData.ftCreationTime.dwHighDateTime;

	LARGE_INTEGER llModifyTime= { 0 };
	llModifyTime.LowPart = fileData.ftLastWriteTime.dwLowDateTime;
	llModifyTime.HighPart = fileData.ftLastWriteTime.dwHighDateTime;


	info.ullFileSize = llSize.QuadPart;
	info.ullAccessTime = llAccessTime.QuadPart;
	info.ullCreateTime = llCreateTime.QuadPart;
	info.ullMondifyTime = llModifyTime.QuadPart;
	info.dwFileAttr = fileData.dwFileAttributes;

	return S_OK;
}

STDMETHODIMP CBaseDevice::SetFileMark(LPCWSTR lpszFile, DWORD dwLeave)
{
	//在64位系统上得特殊处理一下
	DWORD FileReferenceNumber = 0;
	DWORD ParentFileReferenceNumber = 0;
	DWORD dwAttr = 0;
	if (  GetFileReferenceNumber( lpszFile, FileReferenceNumber, ParentFileReferenceNumber,&dwAttr) )
	{
		if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
		{
			DWORD dwIndex = FindDirectory(FileReferenceNumber);
			if ( dwIndex != -1 )
			{
				PFILE_RECORD_INFO d = m_DirPoolIndex[dwIndex];
				if(d) d->BasicInfo.Leave = dwLeave;
			}
			
		}
		else
		{
			DWORD dwIndex = FindFile(FileReferenceNumber);
			if ( dwIndex != -1 )
			{
				PFILE_RECORD_INFO d = m_FilePoolIndex[dwIndex];
				if(d) d->BasicInfo.Leave = dwLeave;
			}
		}
	}
	

	return S_OK;
}

VOID CBaseDevice::OnWorkStatus(LPARAM lParam)
{
	DWORD dwWorkStep = (DWORD)lParam;

	
	//加载数据库
	if ( dwWorkStep == WorkStep_Load )
	{
		UnLockDisk();
		m_dwCurState = DiskState_Scaning;
		m_pDiskSearchNotify->OnDiskSearch_StateChangeNotify(m_cDosDriver, m_dwCurState, 0, 0);

		BOOL bResult = loadCache();
		if ( !bResult )
		{
			GrpError(GroupName, MsgLevel_Error, _T("加载缓存数据库失败:%c"), m_cDosDriver);
			m_Worker.Dispatch(WorkStep_Scan);
			return;
		}

		//加载日志
		bResult = LoadDiskLog();
		if ( !bResult )
		{
			GrpError(GroupName, MsgLevel_Error, _T("扫描日志失败:%c"), m_cDosDriver);
			m_dwCurState = DiskState_Failed;
			m_pDiskSearchNotify->OnDiskSearch_StateChangeNotify(m_cDosDriver, m_dwCurState, 0, 0);
			return;
		}

		//bResult = CacheDisk();

		m_dwCurState = DiskState_Finish;
		m_pDiskSearchNotify->OnDiskSearch_StateChangeNotify(m_cDosDriver, m_dwCurState, 0, 0);
		m_isScanFinesh = TRUE;
	}

	//扫描
	if ( dwWorkStep == WorkStep_Scan )
	{
		UnLockDisk();

		if ( m_dwCurState != DiskState_Scaning)
		{
			m_dwCurState = DiskState_Scaning;
			m_pDiskSearchNotify->OnDiskSearch_StateChangeNotify(m_cDosDriver, m_dwCurState, 0, 0);
		}

		//执行扫描动作
		BOOL bResult = ScanDisk();
		if( !bResult )
		{
			GrpError(GroupName, MsgLevel_Error, _T("扫描磁盘失败:%c"), m_cDosDriver);
			m_dwCurState = DiskState_Failed;
			m_pDiskSearchNotify->OnDiskSearch_StateChangeNotify(m_cDosDriver, m_dwCurState, 0, 0);
			return;
		}

		//到这里就完全成功了
		m_dwCurState = DiskState_Finish;
		m_pDiskSearchNotify->OnDiskSearch_StateChangeNotify(m_cDosDriver, m_dwCurState, 0, 0);
		//bResult = CacheDisk();
		m_isScanFinesh = TRUE;
	}


	//缓存到数据库
	if ( dwWorkStep == WorkStep_Unload)
	{
		m_dwCurState = DiskState_UnKnown;
		
		BOOL bRet = CacheDisk();
		if ( !bRet )
		{
			GrpError(GroupName, MsgLevel_Error, _T("写入缓存失败:%c"), m_cDosDriver);
		}
	}

	return;
}

BOOL CBaseDevice::loadCache()
{
	return TRUE;
}

BOOL CBaseDevice::ScanDisk()
{
	return TRUE;
}

BOOL CBaseDevice::CacheDisk()
{
	return TRUE;
}

BOOL CBaseDevice::LoadDiskLog()
{
	return TRUE;
}

DWORD CBaseDevice::FindDirectory(DWORD dwFileReferenceNumber)
{
	FILE_RECORD_INFO d;
	d.BasicInfo.Index = dwFileReferenceNumber;
	return m_DirPoolIndex.FindInSorted(&d);
}

DWORD CBaseDevice::FindFile(DWORD dwFileReferenceNumber)
{
	FILE_RECORD_INFO d;
	d.BasicInfo.Index = dwFileReferenceNumber;
	return m_FilePoolIndex.FindInSorted(&d);
}


BOOL CBaseDevice::AddDirectoryBase(DWORDLONG frn, DWORDLONG parent_frn, PWCHAR pFileName, int dwFileNameLen, DWORD dwAttr)
{
	INT nParentIndex = FindDirectory((DWORD)parent_frn);
	INT nFileIndex = FindDirectory((DWORD)frn);

	PFILE_RECORD_INFO pParentDir = NULL;
	if (nParentIndex != -1)
	{
		pParentDir = m_DirPoolIndex[nParentIndex];
	}


	if (nFileIndex != -1) //如果存在文件
	{
		PFILE_RECORD_INFO pFile = m_DirPoolIndex[nFileIndex];
		if (pFile->pParentDirRecord != pParentDir) //如果发现父目录不同直接删掉
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("AddDirectoryBase(%d,%d,%s)pFile->pParentDirRecord != pParentDir"), (DWORD)frn, (DWORD)parent_frn, pFileName);
			DestroyFileInfo(&m_Mpool, pFile);

			m_DirPoolIndex.Delete(nFileIndex);
			if (pParentDir == NULL)
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("AddDirectoryBase(%d,%d,%s)pParentDir == NULL"), (DWORD)frn, (DWORD)parent_frn, pFileName);
				return FALSE;
			}
		}
		else
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("AddDirectoryBase(%d,%d,%s)FindFile(frn) != -1"), (DWORD)frn, (DWORD)parent_frn, pFileName);
			return TRUE;
		}

	}

	PFILE_RECORD_INFO pDir = CreateFileInfo(&m_Mpool,JC_FILE_BASIC_INFO(frn).Index, JC_FILE_BASIC_INFO(parent_frn).Index, pFileName, dwFileNameLen,0);
	pDir->pParentDirRecord = pParentDir;

	m_DirPoolIndex.AddToUniqueSorted(pDir);

	if (m_pDiskSearchNotify && m_isScanFinesh )
	{
		TCHAR lpszFullPath[MAX_PATH] = {0};
		GetFullPath(lpszFullPath, pDir);
		m_pDiskSearchNotify->OnDiskSearch_FileChange(m_cDosDriver, lpszFullPath, FileAction_New,dwAttr);
	}

	return TRUE;
}


BOOL CBaseDevice::AddFileBase(DWORDLONG frn, DWORDLONG parent_frn, PWCHAR pFileName, int dwFileNameLen, DWORD dwAttr, BOOL bReName)
{
	INT nFileIndex = FindFile((DWORD)frn);
	INT nParentIndex = FindDirectory((DWORD)parent_frn);


	PFILE_RECORD_INFO pParentDir = NULL;
	if (nParentIndex != -1)
	{
		pParentDir = m_DirPoolIndex[nParentIndex];
	}

	if (nFileIndex != -1) //如果存在文件
	{
		PFILE_RECORD_INFO pFile = m_FilePoolIndex[nFileIndex];
		if (pFile->pParentDirRecord != pParentDir) //如果发现父目录不同直接删掉
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("AddFileBase(%d,%d,%s)pFile->pParentDirRecord != pParentDir"), (DWORD)frn, (DWORD)parent_frn, pFileName);
			DestroyFileInfo(&m_Mpool, pFile);
			m_FilePoolIndex.Delete(nFileIndex);
			if (pParentDir == NULL)
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("AddFileBase(%d,%d,%s)pParentDir == NULL"), (DWORD)frn, (DWORD)parent_frn, pFileName);
				return FALSE;
			}
		}
		else
		{
			GrpError(GroupName,MsgLevel_Error, _T("AddFileBase(%d,%d,%s)FindFile(frn) != -1"), (DWORD)frn, (DWORD)parent_frn, pFileName);
			return TRUE;
		}

	}

	BOOL bHasExten = FALSE;
	INT  nNameLen = 0;
	WORD dwExtenID = 0;

	//计算扩展名
	for (nNameLen = dwFileNameLen - 1; nNameLen >= 0 && pFileName[nNameLen] != L' '&&pFileName[nNameLen] != L'.'; --nNameLen);
	if (nNameLen <= 0 || pFileName[nNameLen] == L' ')
	{
		nNameLen = dwFileNameLen;  //没有长度
	}
	else
	{
		bHasExten = TRUE;
		dwExtenID = m_extenNameMgr.InsertExtenName(pFileName + nNameLen + 1, dwFileNameLen - 1 - nNameLen);
	}


	PFILE_RECORD_INFO pFile = CreateFileInfo(&m_Mpool,
		JC_FILE_BASIC_INFO(frn).Index,
		JC_FILE_BASIC_INFO(parent_frn).Index,
		pFileName, nNameLen ,dwExtenID
		);

	pFile->pParentDirRecord = pParentDir;
	m_FilePoolIndex.AddToUniqueSorted(pFile);

	if (m_pDiskSearchNotify && m_isScanFinesh)
	{
		TCHAR lpszFullPath[MAX_PATH] = {0};
		GetFullPath(lpszFullPath, pFile);
		m_pDiskSearchNotify->OnDiskSearch_FileChange(m_cDosDriver, lpszFullPath, bReName ? FileAction_Rm : FileAction_New,dwAttr);
	}

	return TRUE;
}

BOOL CBaseDevice::RenameNewDirectory(DWORDLONG frn, DWORDLONG parent_frn, PWCHAR pFileName, int nameLen, DWORD dwAttr)
{
	//GrpMsg(GroupName,MsgLevel_Msg, _T("RenameNewDirectory(%d,%d,%s)."), (DWORD)frn, (DWORD)parent_frn, pFileName);
	/*重命名新的文件*/
	INT nOldDirIndex = FindDirectory(m_dwReNameOldFileReferenceNumber);
	m_dwReNameOldFileReferenceNumber = 0;
	if (nOldDirIndex == -1)
	{
		GrpMsg(GroupName,MsgLevel_Msg, _T("RenameNewDirectory(%d,%d,%s).__faild__1"), (DWORD)frn, (DWORD)parent_frn, pFileName);
		return FALSE;
	}

	PFILE_RECORD_INFO pOldDir = m_DirPoolIndex[nOldDirIndex];
	m_DirPoolIndex.Delete(nOldDirIndex);


	PFILE_RECORD_INFO pParentDir = NULL;
	INT nParentIndex = FindDirectory((DWORD)parent_frn);
	if (nParentIndex == -1)
	{
		GrpMsg(GroupName,MsgLevel_Msg, _T("RenameNewDirectory(%d,%d,%s).__faild__3"), (DWORD)frn, (DWORD)parent_frn, pFileName);
		//GrpError(GroupName,MsgLevel_Error, _T("RenameNewDirectory(%d,%d,%s)"), (DWORD)frn, (DWORD)parent_frn, pFileName);
		return FALSE;
	}


	pParentDir = m_DirPoolIndex[nParentIndex];

	PFILE_RECORD_INFO pNewDir = NULL;
	INT nNewDirIndex = FindDirectory((DWORD)frn);
	if (nNewDirIndex != -1)
	{
		pNewDir = m_DirPoolIndex[nNewDirIndex];
		if (pNewDir->pParentDirRecord != pParentDir)
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("RenameNewDirectory(%d,%d,%s).-->pNewDir->pParentDirRecord != pParentDir"), (DWORD)frn, (DWORD)parent_frn, pFileName);
			DestroyFileInfo(&m_Mpool, pNewDir);
			pNewDir = NULL;
			m_DirPoolIndex.Delete(nNewDirIndex);
		}
	}

	if (!pNewDir)
	{
		pNewDir = CreateFileInfo(&m_Mpool, JC_FILE_BASIC_INFO(frn).Index,
			JC_FILE_BASIC_INFO(parent_frn).Index,
			pFileName, nameLen ,0);
	}

	pNewDir->pParentDirRecord = pParentDir;
	PFILE_RECORD_INFO pDir = NULL;

	INT nDirCount = m_DirPoolIndex.Size();
	PFILE_RECORD_INFO* pDirPoint = m_DirPoolIndex.GetPoint();
	for (INT nIndex = 0; nIndex < nDirCount; ++nIndex)
	{
		pDir = pDirPoint[nIndex];
		if (pDir->pParentDirRecord == pOldDir)
		{
			pDir->pParentDirRecord = pNewDir;
		}
	}

	PFILE_RECORD_INFO pFile = NULL;
	INT nFileCount = m_FilePoolIndex.Size();
	PFILE_RECORD_INFO* pFilePoint = m_FilePoolIndex.GetPoint();

	for (INT nIndex = 0; nIndex < nFileCount; nIndex++)
	{
		pFile = pFilePoint[nIndex];
		if (pFile->pParentDirRecord == pOldDir)
		{
			pFile->pParentDirRecord = pNewDir;
		}
	}

	m_DirPoolIndex.AddToUniqueSorted(pNewDir);
	DestroyFileInfo(&m_Mpool, pOldDir);

	if (m_pDiskSearchNotify && m_isScanFinesh)
	{
		TCHAR lpszFullPath[MAX_PATH] = {0};
		GetFullPath(lpszFullPath, pNewDir);
		m_pDiskSearchNotify->OnDiskSearch_FileChange(m_cDosDriver, lpszFullPath, FileAction_New,dwAttr);
	}


	return TRUE;
}

BOOL CBaseDevice::RenameOldDirectory(DWORDLONG frn, PWCHAR fileName, int nameLen, DWORD dwAttr)
{
	if (m_dwReNameOldFileReferenceNumber != 0)
	{
		GrpMsg(GroupName,MsgLevel_Msg, _T("RenameOldDirectory(%d,%d,%s)._m_dwReNameOldFileReferenceNumber!=0"), (DWORD)frn, (DWORD)0, fileName);
	}

	m_dwReNameOldFileReferenceNumber = (DWORD)frn;
	if (m_pDiskSearchNotify && m_isScanFinesh)
	{
		DWORD dwIndex = FindDirectory(m_dwReNameOldFileReferenceNumber);
		if (dwIndex != -1)
		{
			PFILE_RECORD_INFO pDir = m_DirPoolIndex[dwIndex];
			TCHAR lpszFullPath[MAX_PATH] = {0};
			GetFullPath(lpszFullPath, pDir);
			m_pDiskSearchNotify->OnDiskSearch_FileChange(m_cDosDriver, lpszFullPath, FileAction_OldRm,dwAttr);
		}
	}


	return TRUE;
}

BOOL CBaseDevice::DeleteFileBase(DWORDLONG frn, PWCHAR pFileName, int nameLen, BOOL bReName, DWORD dwAttr)
{
	INT nFileIndex = FindFile((DWORD)frn);
	if (nFileIndex != -1)
	{
		PFILE_RECORD_INFO pFile = m_FilePoolIndex[nFileIndex];
		TCHAR lpszFullPath[MAX_PATH] = {0};
		GetFullPath(lpszFullPath, pFile);

		DestroyFileInfo(&m_Mpool, pFile);
		m_FilePoolIndex.Delete(nFileIndex);

		if (m_pDiskSearchNotify && m_isScanFinesh)
		{
			m_pDiskSearchNotify->OnDiskSearch_FileChange(m_cDosDriver, lpszFullPath, bReName ? FileAction_OldRm:FileAction_Del,dwAttr);
		}
	}

	return TRUE;
}

BOOL CBaseDevice::DeleteDirectoryBase(DWORDLONG frn, PWCHAR fileName, int nameLen, DWORD dwAttr)
{

	INT nDirIndex = FindDirectory((DWORD)frn);
	if (nDirIndex == -1)
	{
		GrpMsg(GroupName,MsgLevel_Msg, _T("DeleteDirectoryBase(%d,%d,%s)."), (DWORD)frn, (DWORD)0, fileName);
		return FALSE;
	}

	PFILE_RECORD_INFO pDelDir = m_DirPoolIndex[nDirIndex];


	TCHAR lpszFullPath[MAX_PATH] = {0};
	GetFullPath(lpszFullPath, pDelDir);

	if (nDirIndex != -1)
	{
		m_DirPoolIndex.Delete(nDirIndex);
		DestroyFileInfo(&m_Mpool, pDelDir);
	}

	if (m_pDiskSearchNotify && m_isScanFinesh)
	{
		m_pDiskSearchNotify->OnDiskSearch_FileChange(m_cDosDriver, lpszFullPath, FileAction_Del,dwAttr);
	}


	return TRUE;
}


LPCTSTR CBaseDevice::GetFullPath(LPTSTR lpszFullPath, PFILE_RECORD_INFO pFile)
{
	TCHAR lpszName[MAX_PATH] = {0};
	DWORD dwNameLen = GetFileName(pFile, lpszName);
	
	GetFilePath(pFile, lpszFullPath);


	DWORD nExtenID = GetExtendID(pFile);
	if ( nExtenID )
	{
		LPCWSTR lpszExtenName = L"";
		inline_wcscat(lpszName, L".");
		lpszExtenName = m_extenNameMgr.GetExtenName(nExtenID);
		inline_wcscat(lpszName, lpszExtenName);
	}
	

	inline_wcscat(lpszFullPath,lpszName);
	return lpszFullPath;
}


BOOL CBaseDevice::GetExtIdFromStr(CApiMap<INT, INT>& ExtensionIdMap, PCWCHAR pStr, BOOL bFullCase)
{

	INT nLen = inline_wcslen(pStr);
	if (pStr && nLen)  //说明有包含条件
	{
		INT nBufferLen = (wcslen(pStr) + 1) * 2;
		WCHAR* pPoint = (WCHAR*)mpool_salloc(&m_Mpool, nBufferLen);
		//WCHAR* pPoint = new WCHAR[nLen + 1];
		WCHAR* newStr = pPoint;
		//
		inline_wcscpy(newStr, pStr);
		inline_wcswr(newStr, nLen);

		TCHAR *next_token = NULL;
		TCHAR *szCmd = wcstok_s(newStr, L".|;", &next_token);
		while (szCmd != NULL && _tcslen(szCmd))
		{
			if ( !bFullCase )
			{
				std::vector<int> v =  m_extenNameMgr.Find2(szCmd);
				for ( std::vector<int>::iterator it = v.begin() ; it != v.end() ; it++)
				{
					ExtensionIdMap.Insert(*it, 0);
				}
			}
			else
			{
				INT nPos = m_extenNameMgr.Find(szCmd);
				if ( nPos != -1)
				{
					ExtensionIdMap.Insert(nPos, 0);
				}
				
			}
			
			
			szCmd = _tcstok_s(NULL, L"|;", &next_token);
		}




		//delete[] pPoint;
		mpool_sfree(&m_Mpool, pPoint, nBufferLen);
		//如果在扩展名列表中都不存在，直接返回失败
		return ExtensionIdMap.Count() > 0;
	}

	return TRUE;
}

BOOL CBaseDevice::GetIncludePathFromStr(CApiMap<DWORD, DWORD>& Map, PCWCHAR pStr)
{
	if (pStr && wcslen(pStr))  //说明有包含条件
	{
		//INT nLen = wcslen(pStr) + 1 ;
		//WCHAR* pPoint = new WCHAR[nLen];

		INT nLen = wcslen(pStr) * 2 + 2;
		WCHAR* pPoint = (WCHAR*)mpool_salloc(&m_Mpool, nLen);
		
		WCHAR* newStr = pPoint;
		inline_wcscpy(newStr, pStr);
		inline_wcswr(newStr, inline_wcslen(newStr));
		//看看搜索条件是否为本盘符下的子目录
		TCHAR *next_token = NULL;
		TCHAR *szCmd = wcstok_s(newStr, L"|;", &next_token);
		while (szCmd != NULL && _tcslen(szCmd))
		{
			if (PathIsDirectory(szCmd) && *szCmd == g_NoCaseTable[m_cDosDriver] )
			{
				DWORD FileReferenceNumber = 0;
				DWORD ParentFileReferenceNumber = 0;

				if (GetFileReferenceNumber(szCmd, FileReferenceNumber, ParentFileReferenceNumber))
				{
					Map.Insert(FileReferenceNumber, ParentFileReferenceNumber);
				}
			}
			/*
			HANDLE hFile = CreateFileW(newStr, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

			if (hFile != INVALID_HANDLE_VALUE) //过滤无效的输入
			{
				BY_HANDLE_FILE_INFORMATION fileInfo = { 0 };
				GetFileInformationByHandle(hFile, &fileInfo);
				CloseHandle(hFile);

				//必须是目录,这里可以做去掉重复的优化，以后再做吧
				if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && *szCmd == m_cDosDriver && nCount < nMax)
				{
					pReferenceNumber[nCount] = DWORD(fileInfo.nFileIndexLow);
					nCount++;
				}
			}
			*/
			szCmd = _tcstok_s(NULL, L"|;", &next_token);
		}
		mpool_sfree(&m_Mpool, pPoint, nLen);
		//delete[] pPoint;
		
		return Map.Count() > 0 ? TRUE : FALSE;
	}

	return TRUE;
}

BOOL CBaseDevice::GetFileReferenceNumber(LPCTSTR lpszPath, DWORD& FileReferenceNumber, DWORD& ParentFileReferenceNumber, DWORD* dwAttr)
{
	if(wcslen(lpszPath) <= 2 && lpszPath[1] == ':')
	{
		FileReferenceNumber = 5;
		ParentFileReferenceNumber = 0;
		return TRUE;
	}

	HANDLE hFile = CreateFileW(lpszPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (hFile != INVALID_HANDLE_VALUE) //过滤无效的输入
	{
		BY_HANDLE_FILE_INFORMATION fileInfo = { 0 };
		GetFileInformationByHandle(hFile, &fileInfo);
		CloseHandle(hFile);

		FileReferenceNumber = JC_FILE_BASIC_INFO(fileInfo.nFileIndexLow).Index;
		if ( dwAttr ) *dwAttr = fileInfo.dwFileAttributes;

		return TRUE;
	}

	return FALSE;
}