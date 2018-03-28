// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include"fSearchDef.h"
#include "mpool/mpool.h"
#include "NtfsDisk.h"
#include "DiskSearchSrv.h"
#include "DiskSearchCli.h"
BYTE g_NoCaseTable[ALPHABETA_SIZE] = { 0 };
/*初始化 g_NoCaseTable */
static struct sInitNoCaseTable
{
	sInitNoCaseTable()
	{
		for (int i = 0; i < ALPHABETA_SIZE; ++i){
			if (i >= 'A' && i <= 'Z') g_NoCaseTable[i] = i + 32;
			else g_NoCaseTable[i] = i;
		}
		//g_NoCaseTable[':'] = ':';
	}
} g_sInitNoCaseTable;

USE_DEFAULT_DLL_MAIN;
BEGIN_CLIDMAP
	CLIDMAPENTRY_BEGIN
		CLIDMAPENTRY(CLSID_Ntfs,CNtfsDisk)
		CLIDMAPENTRY(CLSID_DiskSearchSrv,CDiskSearchSrv)
		CLIDMAPENTRY(CLSID_DiskSearchCli, CDiskSearchCli)
		//CLIDMAPENTRY(CLSID_Fat,CNtfsDisk)
	CLIDMAPENTRY_END
END_CLIDMAP
DEFINE_ALL_EXPORTFUN

//mpool m_Mpool;

//HRESULT ScanNtfsDisk(HANDLE hDisk, )

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
LPTSTR /*lpCmdLine*/, int nShowCmd)
{
// 	static const INT MAX_RecvBuffer = sizeof(DWORDLONG)+1024 * 1024 ;
// 	PBYTE RecvBuffer = new BYTE[MAX_RecvBuffer];
// 
// 	MFT_ENUM_DATA med = { 0, 0, m_ntfsDriver.m_curNextUSN };
// 	DWORD cbRet = 0, dwMemRecord = 0;
// 	PUSN_RECORD pRecord = NULL, pEnd = NULL;
// 	PWCHAR pFileName = NULL;
// 	
// 	ZeroMemory(&m_Mpool, sizeof(m_Mpool));
// 	mpool_init_default(&m_Mpool, 4196, 1024);


 	TCHAR sName[] = {L"AAAAAAAAAAAAA马国军AAAAAAAAAA"};
 	PFILE_RECORD_INFO pFile = CreateFileInfo(NULL, 10000, 10000, sName, _tcslen(sName), 1000);

	TCHAR strName[MAX_PATH] = { 0 };
	GetFileName(pFile, strName);
	GetFileNameNoCase(pFile, strName);
	DWORD d = GetFileMemLenght(pFile);
	return 0;
}