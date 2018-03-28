#pragma once

#pragma once

#include "mpool/mpool.h"

#include <util/mpoolvector.h>
#include <util/mpoolmap.h>
#include <SyncObject/rwlock.h>
#include <mscom/srvbase.h>

#include <filemonitor/IFileMonitor.h>
#include <IfSearch.h>
using namespace msdk;

#include "ExtenNameMgr.h"

#include "IfSearch.h"
#include "mscomhelper/UseConnectionPoint.h"
#include "SyncObject/AsynFunc.h"
#include "util/msmap.h"
#include <thread/worker.h>

#include "fSearchDef.h"

class CBaseDevice:
	public IDiskSearch,
	public IOnFileMonitorConnectPoint,
	public worker::IWorkerDispatcher,
	public CMsComBase<CBaseDevice>
{
public:
	CBaseDevice();
	~CBaseDevice();

public:

	UNKNOWN_IMP2_(IDiskSearch, IOnFileMonitorConnectPoint);
	//IDiskSearch
	STDMETHOD(OpenDisk)(WCHAR cDosName, IDiskSearchNotify* pNotify, DWORD/*OpenDiskMask*/ openMask);
	STDMETHOD(CloseDisk)();

	STDMETHOD(LoadDisk)();
	STDMETHOD(UnLoadDisk)();

	STDMETHOD_(WCHAR,GetVolumeName)();
	STDMETHOD_(DWORD, GetCurrentState)();

	STDMETHOD(LockDisk)();
	STDMETHOD(UnLockDisk)();
	STDMETHOD_(BOOL, IsLockDisk)();

	STDMETHOD(Query)(const QueryCondition& Condition, IMsDwordVector* pDwordVector);

	STDMETHOD(GetResultAt)( DWORD dwIndex, DWORD GetValueMask,SearchResult& info);
	STDMETHOD_(DWORDLONG, GetCount)(DWORD* dwFileCount, DWORD* dwDirCount);

	/*
		标记文件，一共有7个等级，0......7,0表示正常
	*/
	STDMETHOD(SetFileMark)(LPCWSTR lpszFile, DWORD dwLeave);


protected:	//IOnFileMonitorConnectPoint
	STDMETHOD(OnFileChangeNotify)(DWORD dwMask, LPCWSTR lpName, DWORD dwAttribute);
	STDMETHOD(OnFileChangeTimeOut)(DWORD dwTimer);
	STDMETHOD(OnAppendMonitor)(LPCWSTR lpVolumes, BOOL succeed) ;
	STDMETHOD(OnRemoveMonitor)(LPCWSTR szVolumes, BOOL succeed) ;


protected://IWorkerDispatcher
	virtual VOID OnWorkStatus(  LPARAM lParam);


protected:
	virtual BOOL loadCache();
	virtual BOOL ScanDisk();
	virtual BOOL CacheDisk();
	virtual BOOL LoadDiskLog();

	virtual HRESULT GetFileInformation(DWORD dwFileID, int* ftCreationTime ,int* ftLastAccessTime, int* ftLastWriteTime, UINT* size, LONGLONG* attr = NULL){return S_OK;};
	virtual BOOL GetFileReferenceNumber(LPCTSTR lpszPath, DWORD& FileReferenceNumber, DWORD& ParentFileReferenceNumber, DWORD* dwAttr = NULL);
public:
	//获取缓存目录
	LPCTSTR GetDataPath();
	LPCTSTR GetFullPath(LPTSTR lpszFullPath, PFILE_RECORD_INFO pDir);

protected:
	BOOL AddDirectoryBase(DWORDLONG frn, DWORDLONG parent_frn, PWCHAR pFileName, int nameLen, DWORD dwAttr);
	BOOL AddFileBase(DWORDLONG frn, DWORDLONG parent_frn, PWCHAR pFileName, int nameLen, DWORD dwAttr, BOOL bReName);
	BOOL RenameNewDirectory(DWORDLONG frn, DWORDLONG parent_frn, PWCHAR pFileName, int nameLen, DWORD dwAttr);
	BOOL RenameOldDirectory(DWORDLONG frn, PWCHAR fileName, int nameLen, DWORD dwAttr);
	BOOL DeleteFileBase(DWORDLONG frn, PWCHAR pFileName, int nameLen, BOOL bReName, DWORD dwAttr);
	BOOL DeleteDirectoryBase(DWORDLONG frn, PWCHAR fileName, int nameLen, DWORD dwAttr);


private:
	DWORD FindDirectory(DWORD dwFileReferenceNumber);
	DWORD FindFile(DWORD dwFileReferenceNumber);
	BOOL  GetExtIdFromStr(CApiMap<INT, INT>& ExtensionIdMap, PCWCHAR pStr, BOOL bFullCase);
	BOOL  GetIncludePathFromStr(CApiMap<DWORD,DWORD>& Map, PCWCHAR pSt);
public:
	TCHAR		m_cDosDriver;
	DWORD		m_OpenDiskMask;

	mpool		m_Mpool; //內存池


	CReadWriteLock	m_rwQueryLock;
	DeclareDefaultAsynFunc;  //退出锁

	CExtenNameMgr					m_extenNameMgr;

	DWORD	m_dwReNameOldFileReferenceNumber;
		
	BOOL m_isScanFinesh;
	BOOL m_bWriteDb;
	BOOL m_bLockDisk;
	BOOL m_bCancle;
	int m_dwCurState;
	TCHAR m_strDataPath[ MAX_PATH ];

	CPoolVectorBase<PFILE_RECORD_INFO>				m_DirPoolIndex; //目录索引
	CPoolVectorBase<PFILE_RECORD_INFO>				m_FilePoolIndex;//文件索引


public:
	UTIL::com_ptr<IDiskSearchNotify>			m_pDiskSearchNotify;
	UseConnectPoint<IOnFileMonitorConnectPoint> m_UseFileMonitorConnectPoint;
	UTIL::com_ptr<IFileMonitor>					m_pFileMonitorService;
	worker::CWorkCenter							m_Worker;
public:
	DWORD		m_dwVolumeSerialNumber;
};
