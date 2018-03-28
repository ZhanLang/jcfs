#pragma once

#include <IfSearch.h>
#include <mscom/srvbase.h>
#include <syncobject/criticalsection.h>
#include <combase/IMsVector.h>
#include <mscomhelper\UseConnectionPoint.h>
#include <SyncObject\criticalsection.h>

#include <map>
struct DiskObject
{
	DiskObject()
	{
		dwFileCount	 = 0;
		dwDirCount	 = 0;
		hSearch		 = 0;
		hFinish		 = 0;
		hExit		 = 0;
		hThread		 = 0;
	}
	DWORD  dwFileCount;
	DWORD  dwDirCount;
	HANDLE hSearch;
	HANDLE hFinish;
	HANDLE hExit;
	HANDLE hThread;
	UTIL::com_ptr<IDiskSearch> pObject;
};


class CQueryCondition
	:public QueryCondition
{
public:
	CQueryCondition(){}
	CQueryCondition(QueryCondition& query)
	{
		strIncludeDir = query.lpstrIncludeDir;
		lpstrIncludeDir = strIncludeDir.GetBuffer();

		strExtension  = query.lpstrExtension;
		lpstrExtension= strExtension.GetBuffer();

		strFileName   = query.lpstrFileName;
		lpstrFileName = strFileName.GetBuffer();

		dwConditionMask = query.dwConditionMask;
	}

	BOOL IsConditionChange(const QueryCondition& query)
	{
		//查询掩码发生变化
		RASSERT(query.dwConditionMask == this->dwConditionMask, TRUE);

		//查询大小发生变化
		RASSERT(query.dwSize == this->dwSize, TRUE);

		//扩展名发生变化
		RASSERT(this->strExtension.CompareNoCase(query.lpstrExtension) == 0, TRUE);
		RASSERT(this->strIncludeDir.CompareNoCase(query.lpstrIncludeDir) == 0, TRUE);

		return this->dwConditionMask & QueryConditionMask_Case ?
			this->strFileName.Compare(query.lpstrFileName) != 0 :
		this->strFileName.CompareNoCase(query.lpstrFileName) != 0;
	}

public:
	CStringW strIncludeDir;
	CStringW strExtension;
	CStringW strFileName;
};

class CDiskSearchCli : 
	public IDiskSearchCli,
	public IDiskSearchNotify,
	public CMsComBase<CDiskSearchCli>
{
public:

	UNKNOWN_IMP2_(IDiskSearchCli, IDiskSearchNotify);
	CDiskSearchCli(void);
	~CDiskSearchCli(void);

	//IDiskSearchCli
	STDMETHOD(Init)(IDiskSearchNotify* pClientNotify);
	STDMETHOD(UnInit)();
	STDMETHOD(Query)(QueryCondition& queryCondition);
	STDMETHOD(GetResultAt)(DWORD dwIndex, DWORD GetValueMask, SearchResult& ResultInfo);
	STDMETHOD_(DWORD, GetState)();
	//IDiskSearchNotify
	STDMETHOD(OnDiskSearch_FileChange)(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr);
	STDMETHOD(OnDiskSearch_Progress)(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur);
	STDMETHOD(OnDiskSearch_FileCountChange)(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount);
	STDMETHOD(OnDiskSearch_StateChangeNotify)(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(OnDiskSearch_Result)( DWORD dwCount, DWORD dwTickCount);
private:
	HRESULT CreateQueryThread();
	HRESULT DestoryQueryThread();

	HRESULT CreateDiskObject(WCHAR cDosName);
	HRESULT DestoryDiskObject(WCHAR cDosName);
private:
	static DWORD WINAPI  _QueryThreadFunction(LPVOID lpParam);
	static DWORD WINAPI  _ObjQueryThreadFunction(LPVOID lpParam);
private:
	HRESULT OnQuery();
	HRESULT OnClientQuery(WCHAR cDosName);

	virtual STDMETHODIMP SetFileMark(LPCWSTR lpszFile, DWORD dwLeave);

private:
	UTIL::com_ptr<IDiskSearchSrv>						m_pFileSearchSrv;
	UTIL::com_ptr<IDiskSearchNotify>					m_pFileSearchClientNotify;
	UTIL::com_ptr<IMsDwordVector>						m_pResult;
private:
	UseConnectPoint<IDiskSearchNotify>					m_UseFileSearchSrvNotifyConnectPoint;
private:
	DECLARE_AUTOLOCK_CS(m_diskMap);	
	typedef std::map<WCHAR, DiskObject> CDiskMap;
	CDiskMap	m_diskMap;
	CQueryCondition	m_queryCondition;

	HANDLE m_hQueryEvent;
	HANDLE m_hExit;
	HANDLE m_hQueryThread;
};

