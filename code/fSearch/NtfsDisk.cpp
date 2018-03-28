#include "StdAfx.h"
#include "NtfsDisk.h"
#include <winioctl.h>
#include <util\memmapfile.h>

CNtfsDisk::CNtfsDisk(void)
{
	m_hVolume = INVALID_HANDLE_VALUE;
	m_curNextUSN = 0;
	m_curJournalID = 0;
}


CNtfsDisk::~CNtfsDisk(void)
{
}


struct DB_HEADER
{
	DWORD		dwSize;
	DWORD		dwVer;
	LONGLONG	llCurUsn;
	DWORD		dwCount;
	DWORD		dwMemLen;
	DWORDLONG	dwCurJournalID;//卷ID
	DWORDLONG	dwTime;
	DWORDLONG	dwTotalCount;
	BYTE		data[1];
};

STDMETHODIMP CNtfsDisk::LockDisk()
{
	SAFE_CLOSEHANDLE(m_hVolume);
	return S_OK;
}

STDMETHODIMP CNtfsDisk::UnLockDisk()
{
	if ( m_hVolume != INVALID_HANDLE_VALUE )
	{
		return TRUE;
	}

	WCHAR szVolumePath[8];
	swprintf_s(szVolumePath, _countof(szVolumePath), L"\\\\.\\%c:", m_cDosDriver);

	m_hVolume=CreateFileW(szVolumePath,
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if ( m_hVolume != INVALID_HANDLE_VALUE )
	{
		return TRUE;
	}

	return FALSE;
}

STDMETHODIMP_(BOOL) CNtfsDisk::IsLockDisk()
{
	if ( m_hVolume != INVALID_HANDLE_VALUE )
		return FALSE;
	

	return TRUE;
}

BOOL CNtfsDisk::loadCache()
{
	CAutoWriteLock readLock(m_rwQueryLock);
	InitVolumeJournal();
	CPoolMap<DWORD, PFILE_RECORD_INFO> DirMap;


	if ( !m_extenNameMgr.LoadFromDatabase() )
	{
		GrpMsg(GroupName,MsgLevel_Msg, _T("<盘符:%c>读取扩展名索引日志失败"), m_cDosDriver);
		return FALSE;
	}


	DWORD dwTickCount = GetTickCount();
	do 
	{
		
		TCHAR szExtPath[MAX_PATH] = { 0 };
		_stprintf_s(szExtPath, MAX_PATH, _T("%s\\dir-%u.dat"), GetDataPath(),m_dwVolumeSerialNumber);

		CMemMapFile memMapFile;
		RASSERT(memMapFile.MapFile(szExtPath, TRUE), FALSE);

		DWORD		dwReadLen = 0;
		DWORD dwFileSize = memMapFile.GetLength();
		

		PBYTE pFileBuffer = (PBYTE)memMapFile.GetBuffer();
		DB_HEADER* pHeader			= (DB_HEADER*)pFileBuffer;

		



		if ( pHeader->dwSize != sizeof(DB_HEADER ))
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取目录数据库的结构头和现有版本不一致"), m_cDosDriver);
			return FALSE;
		}


		if ( pHeader->dwMemLen != dwFileSize)
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取目录数据库的内存长度和记录值不同！"), m_cDosDriver);
			return FALSE;
		}


		if ( pHeader->dwVer != GetCoreVersions() )
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取缓存数据库版本信息不对称[%d] --> %d %d."), m_cDosDriver, pHeader->dwVer, GetCoreVersions());
			return FALSE;
		}

		if ( pHeader->dwCurJournalID != m_curJournalID )
		{
			GrpError(GroupName,MsgLevel_Error, _T("当前记录卷ID于实时获取到的ID不同{%I64d}  {%I64d} DOSNAME:%c"),pHeader->dwCurJournalID, m_curJournalID, m_cDosDriver);
			return FALSE;
		}

		if (pHeader->llCurUsn < m_curFirstUSN  )
		{
			GrpError(GroupName,MsgLevel_Error, _T("当前记录卷NextUSN{%I64d}小于 当前 {%I64d} , DOSNAME:%c"),pHeader->llCurUsn ,m_curFirstUSN, m_cDosDriver);
			return FALSE;
		}

		if ( pHeader->dwTotalCount > ( m_dwlTotalCount + 2000) )
		{
			GrpError(GroupName,MsgLevel_Error, _T("盘符[%c] 当前的文件总数小于磁盘上的文件总数"), m_cDosDriver);
			return FALSE;
		}

		DWORD dwDirCount = pHeader->dwCount;

		if ( dwDirCount == 0 )
		{
			return FALSE;
		}

		pFileBuffer = pHeader->data;

		for (DWORD dwDirIndex = 0; dwDirIndex < dwDirCount; ++dwDirIndex)
		{
			PFILE_RECORD_INFO ptempDir = (PFILE_RECORD_INFO)pFileBuffer;
			DWORD dwRecordLen = GetFileMemLenght(ptempDir);
			PFILE_RECORD_INFO pDir = (PFILE_RECORD_INFO)mpool_salloc(&m_Mpool, dwRecordLen);
			CopyMemory(pDir, ptempDir, dwRecordLen);
			DirMap.Insert(pDir->BasicInfo.Index, pDir);
			pFileBuffer += dwRecordLen;
		}

		//整理目录
		PFILE_RECORD_INFO pParent = NULL, pDir = NULL;
		CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator itDir = DirMap.Min();
		for (; itDir ; itDir++)
		{
			pDir = itDir->Value;
			CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator it = DirMap.Find(JC_FILE_BASIC_INFO(pDir->dwParentFileReferenceNumber).Index);
			if ( it )
			{
				pParent = it->Value;
				pDir->pParentDirRecord = pParent;
			}
			else
			{
				TCHAR szName[ MAX_PATH ] = { 0 };
				GetFileName(pDir, szName);
				if ( pDir->BasicInfo.Index != 5)
					GrpMsg(GroupName, MsgLevel_Msg, _T("整理目录发现有父节点为空的项[%c]：%s"),m_cDosDriver, szName);
			}
			
			m_DirPoolIndex.AddToUniqueSorted(pDir);
		}
		memMapFile.UnMap();
	} while (FALSE);


	do 
	{
		TCHAR szExtPath[MAX_PATH] = { 0 };
		_stprintf_s(szExtPath, MAX_PATH, _T("%s\\file-%u.dat"), GetDataPath(),m_dwVolumeSerialNumber);

		CMemMapFile memMapFile;
		RASSERT(memMapFile.MapFile(szExtPath, TRUE), FALSE);

		DWORD		dwReadLen = 0;
		DWORD dwFileSize = memMapFile.GetLength();
		if ( dwFileSize <= sizeof(DB_HEADER))
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取文件缓存数据库文件大小小于数据头结构[%d] --> %d %d."), m_cDosDriver);
			return FALSE;
		}

		PBYTE pFileBuffer = (PBYTE)memMapFile.GetBuffer();
		DB_HEADER* pHeader			= (DB_HEADER*)pFileBuffer;


		

		if ( pHeader->dwSize != sizeof(DB_HEADER ))
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取文件数据库的结构头和现有版本不一致"), m_cDosDriver);
			return FALSE;
		}

		if ( pHeader->dwMemLen != dwFileSize)
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取文件数据库的内存长度和记录值不同！"), m_cDosDriver);
			return FALSE;
		}

		if ( pHeader->dwVer != GetCoreVersions() )
		{
			GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 读取缓存数据库版本信息不对称[%d] --> %d %d."), m_cDosDriver, pHeader->dwVer, GetCoreVersions());
			return FALSE;
		}

		if ( pHeader->dwCurJournalID != m_curJournalID )
		{
			GrpError(GroupName,MsgLevel_Error, _T("当前记录卷ID于实时获取到的ID不同{%I64d}  {%I64d} DOSNAME:%c"),pHeader->dwCurJournalID, m_curJournalID, m_cDosDriver);
			return FALSE;
		}

		if (pHeader->llCurUsn < m_curFirstUSN  )
		{
			GrpError(GroupName,MsgLevel_Error, _T("当前记录卷NextUSN{%I64d}小于 当前 {%I64d} , DOSNAME:%c"),pHeader->llCurUsn ,m_curFirstUSN, m_cDosDriver);
			return FALSE;
		}


		if ( pHeader->dwTotalCount > ( m_dwlTotalCount + 2000) )
		{
			GrpError(GroupName,MsgLevel_Error, _T("盘符[%c] 当前的文件总数小于磁盘上的文件总数"), m_cDosDriver);
			return FALSE;
		}


		DWORD dwDirCount = pHeader->dwCount;

		if ( dwDirCount == 0 )
		{
			return FALSE;
		}

		pFileBuffer = pHeader->data;

		for (DWORD dwDirIndex = 0; dwDirIndex < dwDirCount; dwDirIndex++)
		{
			PFILE_RECORD_INFO ptempDir = (PFILE_RECORD_INFO)pFileBuffer;
			DWORD dwRecordLen = GetFileMemLenght(ptempDir);
			PFILE_RECORD_INFO pDir = (PFILE_RECORD_INFO)mpool_salloc(&m_Mpool, dwRecordLen);
			CopyMemory(pDir, ptempDir, dwRecordLen);
			
			CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator it = DirMap.Find(JC_FILE_BASIC_INFO(pDir->dwParentFileReferenceNumber).Index);
			if ( it )
			{
				pDir->pParentDirRecord = it->Value;
				m_FilePoolIndex.AddToUniqueSorted(pDir);
			}
			else
			{
				TCHAR szName[ MAX_PATH ] = { 0 };
				GetFileName(pDir, szName);
				if ( pDir->BasicInfo.Index != 5)
					GrpMsg(GroupName, MsgLevel_Msg, _T("整理文件时发现有父节点为空的项[%c]：%s"),m_cDosDriver, szName);
			}

			pFileBuffer += dwRecordLen;
		}

		//m_curJournalID = pHeader->dwCurJournalID;
		m_curNextUSN = pHeader->llCurUsn; 
	} while (FALSE);


	dwTickCount = GetTickCount() - dwTickCount;
	GrpMsg(GroupName, MsgLevel_Msg, _T("读取日志消耗[%c]：%d"),m_cDosDriver, dwTickCount);
	return TRUE;
}

BOOL CNtfsDisk::ScanDisk()
{
	InitVolumeJournal();
	CAutoWriteLock readLock(m_rwQueryLock);


	static const INT MAX_RecvBuffer = sizeof(DWORDLONG)+1024 * 1024 ;
	PBYTE RecvBuffer = new BYTE[MAX_RecvBuffer];

	MFT_ENUM_DATA med = { 0, 0, m_curNextUSN };

	DWORD cbRet = 0, dwMemRecord = 0;
	PUSN_RECORD pRecord = NULL, pEnd = NULL;
	PWCHAR pFileName = NULL;

	DWORD dwFileNameLen = 0;	 /*文件名长度*/
	DWORD dwIconIndex = 0XFFFF;  /*图标序号*/
	WORD  dwExtenID = 0;		 /*扩展名序号*/
	INT	  nNameLen = 0;
	DWORD dwTick = GetTickCount();

	//缓存
	CPoolMap<DWORD, PFILE_RECORD_INFO>	DirPoolMap;   //目录集合
	CPoolMap<DWORD, PFILE_RECORD_INFO>	FilePoolMap;	//文件集合

	m_DirPoolIndex.Clear();
	m_FilePoolIndex.Clear();

	UINT nSize = 0;
	int nAccessTime = 0;

	//插入根节点
	TCHAR strDrivers[MAX_PATH] = { 0 };
	_stprintf_s(strDrivers, MAX_PATH, _T("%c:"), m_cDosDriver);
	PFILE_RECORD_INFO pFile = CreateFileInfo(&m_Mpool, 5, 0, strDrivers, 2, 0);
	DirPoolMap.Insert(5, pFile);


	//开始扫描
	DWORDLONG dwTotalCount = m_dwlTotalCount + 1000;
	DWORDLONG dwCurCount = 0;

	while (TRUE)
	{
		if (!DeviceIoControl(m_hVolume, FSCTL_ENUM_USN_DATA,&med, sizeof(med),RecvBuffer, MAX_RecvBuffer, &cbRet,NULL))
		{
			if ( GetLastError() == ERROR_HANDLE_EOF)
				break;
			return FALSE;
		}

		for (	pRecord = (PUSN_RECORD)&RecvBuffer[sizeof(USN)], pEnd = PUSN_RECORD(RecvBuffer + cbRet);
				pRecord < pEnd ;
				pRecord = (PUSN_RECORD)((PBYTE)pRecord + pRecord->RecordLength))
		{
			pFileName = pRecord->FileName;
			dwFileNameLen = pRecord->FileNameLength / 2;

			if (pRecord->FileAttributes&FILE_ATTRIBUTE_DIRECTORY /*&& !(pRecord->FileAttributes & FILE_ATTRIBUTE_SYSTEM)*/)
			{
				dwCurCount ++;
				pFile = CreateFileInfo(&m_Mpool,
					JC_FILE_BASIC_INFO(pRecord->FileReferenceNumber).Index,
					JC_FILE_BASIC_INFO(pRecord->ParentFileReferenceNumber).Index
					,pRecord->FileName, dwFileNameLen,0);

				if ( pFile )
				{
					DirPoolMap.Insert(JC_FILE_BASIC_INFO(pRecord->FileReferenceNumber).Index, pFile);
				}
			}
			else  if ((pRecord->FileAttributes != 0xFFFFFFFF) && (!(pRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
			{
				dwCurCount ++;
				nNameLen = 0;
				BOOL bHasExten = FALSE;
				dwExtenID = 0;

				for (nNameLen = dwFileNameLen - 1; nNameLen >= 0 && pFileName[nNameLen] != L' '&&pFileName[nNameLen] != L'.'; --nNameLen);
				if (nNameLen <= 0 || pFileName[nNameLen] == L' ')
					nNameLen = dwFileNameLen;  //没有长度
				else
				{
					bHasExten = TRUE;
					dwExtenID = m_extenNameMgr.InsertExtenName(pFileName + nNameLen + 1, dwFileNameLen - 1 - nNameLen);
				}

				pFile = CreateFileInfo(&m_Mpool,JC_FILE_BASIC_INFO(pRecord->FileReferenceNumber).Index,
					JC_FILE_BASIC_INFO(pRecord->ParentFileReferenceNumber).Index,
					pRecord->FileName, nNameLen ,dwExtenID);

				FilePoolMap.Insert(JC_FILE_BASIC_INFO(pRecord->FileReferenceNumber).Index, pFile);
			}

			if (dwCurCount % 100 == 0)
				m_pDiskSearchNotify->OnDiskSearch_Progress(m_cDosDriver, dwTotalCount,dwCurCount);
		}
		med.StartFileReferenceNumber = *(DWORDLONG*)RecvBuffer;
	}

	dwTick = GetTickCount() - dwTick;
	GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 建立文件红黑树耗时[%d]共扫描[%d]个文件."), m_cDosDriver, dwTick, DirPoolMap.Count() + FilePoolMap.Count());



	//整理目录
	dwTick = GetTickCount();
	m_DirPoolIndex.Reserve(DirPoolMap.Count());  //为了提速


	PFILE_RECORD_INFO pParent = NULL;
	for (CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator itDir = DirPoolMap.Min();  itDir  ; itDir++)
	{
		pFile = itDir->Value;
		if ( !pFile )
			break;


		CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator it = DirPoolMap.Find(pFile->dwParentFileReferenceNumber);
		if (it)  //如果父目录存在
		{
			pParent = it->Value;
			if (pParent)
			{
				pFile->pParentDirRecord = pParent;
				m_DirPoolIndex.AddToUniqueSorted(pFile);
			}
			else
			{
				if (pFile->BasicInfo.Index != 5)
				{
					DestroyFileInfo(&m_Mpool, itDir->Value);
					itDir->Value = NULL;
				}
			}

		}
		else
		{
 			if (pFile->BasicInfo.Index != 5)
 			{
				TCHAR szName[ MAX_PATH ] = { 0 };
				GetFileName(pFile, szName);
				GrpMsg(GroupName, MsgLevel_Msg, _T("扫描目录时发现没有父节点的情况[%c]：%s"),m_cDosDriver, szName);

				DestroyFileInfo(&m_Mpool, itDir->Value);
				itDir->Value = NULL;
			}
			else if ( pFile->BasicInfo.Index == 5)
			{
				m_DirPoolIndex.AddToUniqueSorted(pFile);
			}
		}
	}

	m_FilePoolIndex.Reserve(FilePoolMap.Count());
	CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator itFile = FilePoolMap.Min();
	for (CPoolMap<DWORD, PFILE_RECORD_INFO>::Iterator itFile = FilePoolMap.Min() ; itFile; itFile++)
	{
		pFile = itFile->Value;
		if ( !pFile )
			break;
		
		CPoolMap<DWORD,PFILE_RECORD_INFO>::Iterator it = DirPoolMap.Find(pFile->dwParentFileReferenceNumber);
		if (it)
		{
			pParent = it->Value;
			if (pParent)
			{
				pFile->pParentDirRecord = pParent;
				m_FilePoolIndex.AddToUniqueSorted(pFile);
			}
			else
			{
				DestroyFileInfo(&m_Mpool, pFile);
				itFile->Value = NULL;
			}
		}
		else
		{
			DestroyFileInfo(&m_Mpool, pFile);
			itFile->Value = NULL;
		}
	}

	dwTick = GetTickCount() - dwTick;
	GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 建立文件索引耗时[%d]共插入[%d]个文件."), m_cDosDriver, dwTick, m_FilePoolIndex.Size() + m_DirPoolIndex.Size());

	delete[] RecvBuffer;

	return TRUE;
}




BOOL CNtfsDisk::CacheDisk()
{

	if ( m_bWriteDb )
	{
		return TRUE;
	}

	m_bWriteDb = TRUE;
	if ( !m_extenNameMgr.WriteToDatabase() )
	{
		GrpMsg(GroupName,MsgLevel_Msg, _T("<盘符:%c>写入扩展名索引日志失败"), m_cDosDriver);
		return FALSE;
	}
	
	DWORD dwTick = GetTickCount();
	DWORD dwDirCount = m_DirPoolIndex.Size();
	CPoolMap<PFILE_RECORD_INFO, DWORD>  mapParentPtr2Offset;
	DWORD dwDirMemTotalLen = 0;

	for (DWORD dwDirIndex = 0; dwDirIndex < dwDirCount; ++dwDirIndex)
	{
		PFILE_RECORD_INFO pDir = m_DirPoolIndex[dwDirIndex];
		DWORD dwLen = GetFileMemLenght(pDir);
		dwDirMemTotalLen += dwLen;
		mapParentPtr2Offset.Insert(pDir, pDir->BasicInfo.Index  );
	}

	dwDirMemTotalLen += (sizeof(DB_HEADER));


	TCHAR szExtPath[MAX_PATH] = { 0 };
	_stprintf_s(szExtPath, MAX_PATH, _T("%s\\dir-%u.dat"), GetDataPath(),m_dwVolumeSerialNumber);
	GrpMsg(GroupName,MsgLevel_Msg, _T("<盘符:%c>WirteToDatabase:%s"), m_cDosDriver, szExtPath);
	DeleteFile(szExtPath);
	msapi::CreateDirectoryEx(szExtPath);


	HANDLE hFile = CreateFile(szExtPath, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	RASSERT(hFile != INVALID_HANDLE_VALUE, FALSE);
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = dwDirMemTotalLen; //设置成这个大，单位字节
	BOOL bRet = SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN);
	SetEndOfFile(hFile);
	CloseHandle(hFile);


	do
	{
		CMemMapFile dirMemMapFile;
		RASSERT(dirMemMapFile.MapFile(szExtPath, FALSE, dwDirMemTotalLen), FALSE);

		//写入文件长度
		DWORD dwWrite = dirMemMapFile.GetLength();
		LPBYTE buffer = (LPBYTE)dirMemMapFile.GetBuffer();

		DB_HEADER* pHeader			= (DB_HEADER*)buffer;
		ZeroMemory(pHeader, sizeof(DB_HEADER));
		

		buffer = pHeader->data;

		//写入目录
		for (DWORD dwDirIndex = 0; dwDirIndex < dwDirCount;dwDirIndex ++)
		{
			PFILE_RECORD_INFO pDirRecord = m_DirPoolIndex[dwDirIndex];
			CPoolMap<PFILE_RECORD_INFO,DWORD>::Iterator pParentNode = mapParentPtr2Offset.Find(pDirRecord->pParentDirRecord);
			DWORD dwParentReferenceNum = 0;
			if ( pParentNode )
				dwParentReferenceNum = pParentNode->Value;
			else
			{
				if ( pDirRecord->BasicInfo.Index != 5 )
				{
					TCHAR szName[ MAX_PATH ] = { 0 };
					GetFileName(pDirRecord, szName);
					GrpError(GroupName,MsgLevel_Error, _T("目录写入缓存数据库时没有找到父节点[%c]  %s "), m_cDosDriver, szName);
					return FALSE;
				}
			}

			DWORD dwDirLen = GetFileMemLenght(pDirRecord);  //记录长度
			if ( pDirRecord->BasicInfo.Index == 5 )
				pDirRecord->dwParentFileReferenceNumber = 0;
			else
				pDirRecord->dwParentFileReferenceNumber = dwParentReferenceNum;


			memcpy(buffer, pDirRecord, dwDirLen);
			buffer += dwDirLen;
		}

		pHeader->dwSize = sizeof(DB_HEADER);
		pHeader->dwMemLen			= dirMemMapFile.GetLength();	
		pHeader->dwVer				= GetCoreVersions();
		pHeader->dwCurJournalID		= m_curJournalID;
		pHeader->dwTime				= GetCurrentTime2();
		pHeader->llCurUsn			= m_curNextUSN;
		pHeader->dwCount			= dwDirCount;
		pHeader->dwTotalCount = m_FilePoolIndex.Size() + m_DirPoolIndex.Size();
		dirMemMapFile.Flush();
		dirMemMapFile.UnMap();
		

	} while (FALSE);

	


	do 
	{
		_stprintf_s(szExtPath, MAX_PATH, _T("%s\\file-%u.dat"), GetDataPath(),m_dwVolumeSerialNumber);
		DeleteFile(szExtPath);
		DWORD dwFileRecordCount = m_FilePoolIndex.Size();
		CMemMapFile fileMemMapFile;
		DWORD dwFileMemTotalLen = 0;
		for (DWORD dwFileIndex = 0; dwFileIndex < dwFileRecordCount; dwFileIndex++)
		{
			PFILE_RECORD_INFO pFileRecord = m_FilePoolIndex[dwFileIndex];
			DWORD dwNameLen = GetFileMemLenght(pFileRecord);
			dwFileMemTotalLen += dwNameLen;
		}

		dwDirMemTotalLen += sizeof(DB_HEADER) ;

		HANDLE hFile = CreateFile(szExtPath, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		RASSERT(hFile != INVALID_HANDLE_VALUE, FALSE);
		LARGE_INTEGER liDistanceToMove;
		liDistanceToMove.QuadPart = dwFileMemTotalLen; //设置成这个大，单位字节
		BOOL bRet = SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);


		RASSERT(fileMemMapFile.MapFile(szExtPath, FALSE, dwFileMemTotalLen), FALSE);
		PBYTE buffer = (PBYTE)fileMemMapFile.GetBuffer();
		DB_HEADER* pHeader			= (DB_HEADER*)buffer;
		ZeroMemory( pHeader, sizeof(DB_HEADER) );
		

		buffer = pHeader->data;



		for (DWORD dwFileIndex = 0; dwFileIndex < dwFileRecordCount; dwFileIndex++)
		{
			PFILE_RECORD_INFO pFileRecord = m_FilePoolIndex[dwFileIndex];
			CPoolMap<PFILE_RECORD_INFO,DWORD >::Iterator pParentNode = mapParentPtr2Offset.Find(pFileRecord->pParentDirRecord);
			
			if ( pParentNode != NULL )
			{
				//PFILE_RECORD_INFO pParent = pParentNode->Value;
				DWORD dwFileRecordLen = GetFileMemLenght(pFileRecord);
				pFileRecord->dwParentFileReferenceNumber = pParentNode->Value;
				memcpy(buffer, pFileRecord, dwFileRecordLen);
				buffer += dwFileRecordLen;
			}
			else
			{
				TCHAR szName[ MAX_PATH ] = { 0 };
				GetFileName(pFileRecord, szName);
				GrpError(GroupName,MsgLevel_Error, _T("文件写入缓存数据库时没有找到父节点[%c]  %s "), m_cDosDriver, szName);
				return FALSE;
			}
		}

		pHeader->dwSize = sizeof(DB_HEADER);
		pHeader->dwMemLen			= fileMemMapFile.GetLength();	
		pHeader->dwVer				= GetCoreVersions();
		pHeader->dwCurJournalID		= m_curJournalID;
		pHeader->dwTime				= GetCurrentTime2();
		pHeader->llCurUsn			= m_curNextUSN;
		pHeader->dwCount			= dwFileRecordCount;
		pHeader->dwTotalCount = m_FilePoolIndex.Size() + m_DirPoolIndex.Size();
		fileMemMapFile.Flush();
		fileMemMapFile.UnMap();

	} while (FALSE);


	dwTick = GetTickCount() - dwTick;
	GrpMsg(GroupName,MsgLevel_Msg, _T("盘符[%c] 写入缓存数据库耗时[%d]."), m_cDosDriver, dwTick);

	return TRUE;
}

BOOL CNtfsDisk::LoadDiskLog()
{
	const DWORD SEARCH_TITLE_REASON_FLAG = 
		USN_REASON_FILE_CREATE
		| USN_REASON_FILE_DELETE
		| USN_REASON_RENAME_OLD_NAME
		| USN_REASON_RENAME_NEW_NAME;


	READ_USN_JOURNAL_DATA rujd = { 0 };
	rujd.BytesToWaitFor = 0;
	rujd.ReasonMask = SEARCH_TITLE_REASON_FLAG;
	rujd.ReturnOnlyOnClose = 0;
	rujd.StartUsn = m_curNextUSN;
	rujd.Timeout = 0;
	rujd.UsnJournalID = m_curJournalID;


	DWORD dwBytes = 0, dwRetBytes = 0;
	BYTE Buffer[USN_PAGE_SIZE] = { 0 };
	PUSN_RECORD pRecord = NULL;
	BOOL bChange = TRUE;


	DWORDLONG dwTotalCount = m_dwlTotalCount + 1000;
	DWORD dwCurCount = m_FilePoolIndex.Size() + m_DirPoolIndex.Size();

	while (DeviceIoControl(m_hVolume, FSCTL_READ_USN_JOURNAL, &rujd, sizeof(rujd), Buffer, USN_PAGE_SIZE, &dwBytes, NULL))
	{
		if (dwBytes <= sizeof(USN))
			break;
		

		dwRetBytes = dwBytes - sizeof(USN);//跳过了1个USN，此USN是下一论查询起点
		pRecord = PUSN_RECORD((PBYTE)Buffer + sizeof(USN));
		while (dwRetBytes > 0)//若跳过1个USN后，还有多余字节，则至少有1个记录
		{
			bChange = TRUE;
			dwCurCount ++;

			if (dwCurCount % 10 == 0 && !m_isScanFinesh)
			{
				m_pDiskSearchNotify->OnDiskSearch_Progress(m_cDosDriver,dwTotalCount, dwTotalCount > dwCurCount ? dwCurCount : dwTotalCount);
			}

			//添加日志
			if (!ChangeDataBase(pRecord->FileAttributes, pRecord->Reason, pRecord->FileReferenceNumber,
				pRecord->ParentFileReferenceNumber, PWCHAR(pRecord->FileName), pRecord->FileNameLength >> 1))
			{
				bChange = FALSE;
			}

			dwRetBytes -= pRecord->RecordLength;
			//找下一个记录
			pRecord = (PUSN_RECORD)(((PBYTE)pRecord) + pRecord->RecordLength);
		}
		//更新起始USN
		rujd.StartUsn = m_curNextUSN = *(USN*)Buffer;
	}

	return TRUE;
}



BOOL CNtfsDisk::InitVolumeJournal()
{
	DWORD dwWritten = 0;
	LARGE_INTEGER num;
	NTFS_VOLUME_DATA_BUFFER ntfsVolData = {0};

	RASSERT(DeviceIoControl(m_hVolume,FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &ntfsVolData, sizeof(ntfsVolData), &dwWritten, NULL), FALSE);

	//初始化字节簇
	m_BytesPerCluster=ntfsVolData.BytesPerCluster;
	num.QuadPart = 1024;
	m_dwlTotalCount  = (ntfsVolData.MftValidDataLength.QuadPart/num.QuadPart);
	USN_JOURNAL_DATA ujd = {0};
	if(!QueryUsnJournal(&ujd)){//查询失败
		switch(GetLastError())
		{
		case ERROR_JOURNAL_NOT_ACTIVE:
			CreateUsnJournal(0x800000, 0x100000); 
			QueryUsnJournal(&ujd);
			break;

		case ERROR_JOURNAL_DELETE_IN_PROGRESS:
			DeleteUsnJournal(&ujd);
			CreateUsnJournal(0x2000000,0x400000); 
			QueryUsnJournal(&ujd);
			break;
		}
	}

	m_curJournalID = ujd.UsnJournalID;
	m_curFirstUSN  = ujd.FirstUsn;
	m_curNextUSN   = ujd.NextUsn;

	return TRUE;
}


BOOL CNtfsDisk::QueryUsnJournal(PUSN_JOURNAL_DATA pUsnJournalData)
{
	DWORD cb;
	return DeviceIoControl(m_hVolume,FSCTL_QUERY_USN_JOURNAL,NULL,0, pUsnJournalData,sizeof(USN_JOURNAL_DATA), &cb,NULL);
}


BOOL CNtfsDisk::CreateUsnJournal(DWORDLONG MaximumSize, DWORDLONG AllocationDelta)
{
	DWORD cb;
	CREATE_USN_JOURNAL_DATA cujd;
	cujd.MaximumSize = MaximumSize;
	cujd.AllocationDelta = AllocationDelta;
	return DeviceIoControl(m_hVolume, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &cb, NULL);
}

BOOL CNtfsDisk::DeleteUsnJournal(PUSN_JOURNAL_DATA pUsnJournalData)
{
	DWORD cb;
	DELETE_USN_JOURNAL_DATA del_ujd;
	del_ujd.UsnJournalID = pUsnJournalData->UsnJournalID;
	del_ujd.DeleteFlags = USN_DELETE_FLAG_NOTIFY;

	return DeviceIoControl(m_hVolume, FSCTL_DELETE_USN_JOURNAL, &del_ujd, sizeof(DELETE_USN_JOURNAL_DATA), NULL, 0, &cb, NULL);
}

BOOL CNtfsDisk::ChangeDataBase(DWORD dwAttr, DWORD dwReason, DWORDLONG frn, DWORDLONG parent_frn, PWCHAR _wszFileName, DWORD dwNameLen)
{

	BOOL bDir = dwAttr & FILE_ATTRIBUTE_DIRECTORY;
	BOOL bSuccess = FALSE;

	TCHAR wszFileName[MAX_PATH] = { 0 };
	for (DWORD n = 0; n < dwNameLen; n++)
	{
		wszFileName[n] = _wszFileName[n];
		wszFileName[n + 1] = '\0';
	}


	if ((USN_REASON_FILE_CREATE == dwReason) /*&& (USN_REASON_CLOSE&dwReason)*/)
	{//增
		if (bDir)
		{
			if (!AddDirectoryBase(frn, parent_frn, wszFileName, dwNameLen,dwAttr))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_FILE_CREATE::AddDirectoryBase{%s}失败"), wszFileName);
				return FALSE;
			}
		}
		else
		{
			if (!AddFileBase(frn, parent_frn, wszFileName, dwNameLen,dwAttr,FALSE))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_FILE_CREATE::AddFileBase{%s}失败"), wszFileName);
				return FALSE;
			}
		}

	}//((USN_REASON_CLOSE | USN_REASON_RENAME_NEW_NAME) & dwReason) == (USN_REASON_CLOSE | USN_REASON_RENAME_NEW_NAME)
	else if (dwReason == /*0x80002000*/USN_REASON_RENAME_NEW_NAME)
	{//增
		if (bDir)//重命名目录 仅根目录变化
		{
			if (!RenameNewDirectory(frn, parent_frn, wszFileName, dwNameLen,dwAttr))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_RENAME_NEW_NAME::RenameNewDirectory{%s}失败"), wszFileName);
				return FALSE;
			}
		}
		else
		{
			if (!AddFileBase(frn, parent_frn, wszFileName, dwNameLen,dwAttr,TRUE))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_RENAME_NEW_NAME::AddFileBase{%s}失败"), wszFileName);
				return FALSE;
			}
		}
	}
	else if ((USN_REASON_RENAME_OLD_NAME ==  dwReason))
	{//删
		if (bDir) //重命名目录 仅根目录变化
		{
			if (!RenameOldDirectory(frn, wszFileName, dwNameLen,dwAttr))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_RENAME_OLD_NAME::RenameOldDirectory{%s}失败"), wszFileName);
				return FALSE;
			}
		}
		else
		{
			if (!DeleteFileBase(frn, wszFileName, dwNameLen,TRUE, dwAttr))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_RENAME_OLD_NAME::DeleteFileBase{%s}失败"), wszFileName);
				return FALSE;
			}
		}
	}//((USN_REASON_CLOSE | USN_REASON_FILE_DELETE) & dwReason) == (USN_REASON_CLOSE | USN_REASON_FILE_DELETE)
	else if (dwReason & /*0x80000200*/USN_REASON_FILE_DELETE)
	{//删
		if (bDir)
		{
			if (!DeleteDirectoryBase(frn, wszFileName, dwNameLen,dwAttr))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_FILE_DELETE::DeleteDirectoryBase{%s}失败"), wszFileName);
				return FALSE;
			}
		}
		else
		{
			if (!DeleteFileBase(frn, wszFileName, dwNameLen,FALSE, dwAttr))
			{
				GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_FILE_DELETE::DeleteFileBase{%s}失败"), wszFileName);
				return FALSE;
			}
		}
	}
	else if (dwReason & USN_REASON_OBJECT_ID_CHANGE)
	{
		//GrpMsg(GroupName,MsgLevel_Msg, _T("USN_REASON_OBJECT_ID_CHANGE{%s}"), wszFileName);
	}

	else if (dwReason & USN_REASON_BASIC_INFO_CHANGE)
	{
		/*
		CBasicInfoChangeMap::iterator itFind = m_BasicInfoChangeMap.find(frn);
		if (itFind == m_BasicInfoChangeMap.end())
		{
			RECORD_INFORMATION file_info = {0};
			file_info.parent_frn = parent_frn;
			file_info.dwFileNameLen = dwNameLen;
			file_info.dwAttr = dwAttr;
			CopyMemory(file_info.szFileName, _wszFileName, dwNameLen*sizeof(WCHAR));
			m_BasicInfoChangeMap.insert(m_BasicInfoChangeMap.end(), CBasicInfoChangeMap::value_type(frn, file_info));
		}
		*/
	}
	else
	{
		return TRUE;
	}

	return TRUE;
}
