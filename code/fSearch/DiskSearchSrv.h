#pragma once

#include "mscomhelper\connectionpointhelper.h"
#include "SyncObject\criticalsection.h"
#include "thread\SubThreadImp.h"

#include <map>
#include <set>

#include <IfSearch.h>
#include <filemonitor/IFileMonitor.h>
#include "mscomhelper\UseConnectionPoint.h"

#include "database/genericdb.h"
#include "util/bootValue.h"

struct FileCountCache
{
	DWORD dwFileCount;
	DWORD dwDirCount;
};

struct FileProgressCache
{
	DWORD dwTotalCount;
	DWORD dwCurrentCount;
};

struct DiskStateCache
{
	DWORD dwState;
};

class CDiskSearchSrv:
	public IMsPlugin,
	public IMsPluginRun,
	public IDiskSearchSrv,
	public IDiskSearchNotify,
	public IDiskChangeConnectPoint,
	public CConnectionPointContainerHelper<CDiskSearchSrv>,
	private CUnknownImp
{
public:
	UNKNOWN_IMP6_(IMsPlugin, IMsPluginRun, IDiskSearchSrv, IMsConnectionPointContainer, IDiskChangeConnectPoint, IDiskSearchNotify);
	STDMETHOD(init_class)(IMSBase* pRot, IMSBase* pOuter)
	{
		RASSERT(pRot, E_INVALIDARG);
		m_pRot = pRot;
		return S_OK;
	}


	CDiskSearchSrv(void);
	~CDiskSearchSrv(void);


protected:
	//IMsPlugin
	STDMETHOD(Init)(void*);
	STDMETHOD(Uninit)();

	//IMsPluginRun
	STDMETHOD(Start)();
	STDMETHOD(Stop)();

	//IDiskChangeConnectPoint
	STDMETHOD(OnDiskChange)(DWORD dwCtrlMask,DWORD dwType,LPCWSTR lpName);

	//IDiskSearchNotify
	STDMETHOD(OnDiskSearch_FileChange)(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr);
	STDMETHOD(OnDiskSearch_Progress)(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur);
	STDMETHOD(OnDiskSearch_FileCountChange)(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount);
	STDMETHOD(OnDiskSearch_StateChangeNotify)(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(OnDiskSearch_Result)( DWORD dwCount, DWORD dwTickCount);
	//IFileSearchSrv
	STDMETHOD_(DWORD, GetSearchDisk)(IMSBase /*IProperty2*/** pSerchDisk);
	STDMETHOD(GetSearchDisk)(WCHAR cDosName, IMSBase/*IDiskSearch*/** pDisk);
	STDMETHOD(Lock)();
	STDMETHOD(UnLock)();
	STDMETHOD(SetFileMark)(LPCWSTR lpszFile, DWORD dwLeave);
	STDMETHOD(ReBuildIndex)();
	STDMETHOD_(DWORD,GetState)();

private:
	HRESULT CreateAllDiskSearch();
	HRESULT DestoryAllDiskSearch();
	HRESULT CreateDiskSearch(WCHAR cDosName);
	HRESULT DeleteDiskSearch(WCHAR cDosName);
	BOOL	GetDiskClsid(WCHAR cDosDriver,CLSID* clsid = NULL);

private:
	HRESULT CreateSqliteDB();
	HRESULT DestorySqliteDB();
	HRESULT InitFileMark(WCHAR wDos, IDiskSearch* pDisk);

private:
	UTIL::com_ptr<IMscomRunningObjectTable>	m_pRot;
	CConnectionPointHelper					m_FileSearchSrvNotifyConnectPoint;
	UseConnectPoint<IDiskChangeConnectPoint> m_useDiskChangePoint;

	typedef std::map<WCHAR, UTIL::com_ptr<IDiskSearch>>	CDiskMap;
	typedef std::set<WCHAR>	CDiskSet;

	CDiskMap	m_diskMap;
	DECLARE_AUTOLOCK_CS(disk_map);


	typedef std::map<WCHAR, FileCountCache> CFileCountCacheMap;
	CFileCountCacheMap m_FileCacheMap;
	DECLARE_AUTOLOCK_CS(m_FileCacheMap);

	typedef std::map<WCHAR, FileProgressCache> CFileProgressCacheMap;
	CFileProgressCacheMap m_FileProgressCacheMap;
	DECLARE_AUTOLOCK_CS(m_FileProgressCacheMap);


	typedef std::map<WCHAR, DiskStateCache> CDiskStateCacheMap;
	CDiskStateCacheMap m_DiskStateCacheMap;
	DECLARE_AUTOLOCK_CS(m_DiskStateCacheMap);


	BOOL m_bReBuild;

	UTIL::com_ptr<IGenericDB> m_pGenericDB;

	CString m_strFileMarkTableName;
	CBootValue m_bootValue;
};

