#pragma once

#include "mpool/mpool.h"
#include "BasicDisk.h"
#include <winioctl.h>

struct RECORD_INFORMATION
{
	WCHAR szFileName[MAX_PATH];
	DWORD dwFileNameLen;
	DWORDLONG parent_frn;
	DWORD dwAttr;
};


class CNtfsDisk : public CBaseDevice
{
public:
	CNtfsDisk(void);
	~CNtfsDisk(void);


protected:
	STDMETHOD(LockDisk)();
	STDMETHOD(UnLockDisk)();
	STDMETHOD_(BOOL, IsLockDisk)();

	virtual BOOL loadCache();
	virtual BOOL ScanDisk();
	virtual BOOL CacheDisk();
	virtual BOOL LoadDiskLog();


private:
	/*初始化卷信息*/
	BOOL InitVolumeJournal();
	BOOL QueryUsnJournal(PUSN_JOURNAL_DATA pUsnJournalData);
	BOOL CreateUsnJournal(DWORDLONG MaximumSize, DWORDLONG AllocationDelta);
	BOOL DeleteUsnJournal(PUSN_JOURNAL_DATA pUsnJournalData);

private:
	BOOL ChangeDataBase(DWORD dwAttr, DWORD dwReason, DWORDLONG frn, DWORDLONG parent_frn, PWCHAR _wszFileName, DWORD dwNameLen);

private:
	USN         m_curFirstUSN;
	USN         m_curNextUSN;
	DWORDLONG   m_curJournalID;	
	HANDLE		m_hVolume;

	DWORD       m_BytesPerCluster;//每簇字节数
	ULONGLONG	m_dwlTotalCount;

};

