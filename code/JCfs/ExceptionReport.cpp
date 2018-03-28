// FileZilla - a Windows ftp client
// modify by rising gyb
//

#include "stdafx.h"
#include <dbghelp.h>
#include "ExceptionReport.h"

#include <Tlhelp32.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")


typedef BOOL
(_stdcall *tMiniDumpWriteDump)(
	HANDLE hProcess,
	DWORD ProcessId,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

static tMiniDumpWriteDump		pMiniDumpWriteDump;

// Global class instance
// static CExceptionReport ExceptionReport;

LPTOP_LEVEL_EXCEPTION_FILTER CExceptionReport::m_previousExceptionFilter = NULL;
TCHAR CExceptionReport::m_pDmpFileName[MAX_PATH] = {0};
HANDLE CExceptionReport::m_hDumpFile = INVALID_HANDLE_VALUE;
BOOL CExceptionReport::m_bFirstRun = TRUE;

CExceptionReport::CExceptionReport()
{
	m_bFirstRun = TRUE;

	m_previousExceptionFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	// Retrieve report/dump filenames
	TCHAR FullPath[MAX_PATH] = {0};
	GetModuleFileName(0, FullPath, MAX_PATH);
	::PathRemoveExtension(FullPath);

	MSExecptionTString time = FormatCurrentTimeString();
	_tcscat_s(FullPath, MAX_PATH, time.c_str());


	_tcscpy_s(m_pDmpFileName, MAX_PATH, FullPath);
	_tcscat_s(m_pDmpFileName, MAX_PATH, _T(".dmp"));

}

CExceptionReport::~CExceptionReport()
{
	SetUnhandledExceptionFilter(m_previousExceptionFilter);
}

LONG WINAPI CExceptionReport::UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	if (!m_bFirstRun)
	{
		// Don't generate exception report twice
		if (m_previousExceptionFilter)
			return m_previousExceptionFilter(pExceptionInfo);
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}
	else
		m_bFirstRun = FALSE;

	// Suspend all threads to freeze the current state
	SuspendThreads();
	
	HMODULE hDll = LoadLibrary(_T("dbghelp.dll"));
	if (!hDll)
	{
		if (m_previousExceptionFilter)
			return m_previousExceptionFilter(pExceptionInfo);
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}

	//加载写dmp函数

	pMiniDumpWriteDump		= (tMiniDumpWriteDump)GetProcAddress(hDll, "MiniDumpWriteDump");

	if (pMiniDumpWriteDump == NULL)
	{
		FreeLibrary(hDll);
		if (m_previousExceptionFilter)
			return m_previousExceptionFilter(pExceptionInfo);
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}


	//真正的写dmp文件

	m_hDumpFile = CreateFile(m_pDmpFileName, GENERIC_WRITE, FILE_SHARE_READ,
								0, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH,	0);

	if (m_hDumpFile == INVALID_HANDLE_VALUE)
	{
		TCHAR tmp[MAX_PATH];
		_tcscpy_s(tmp, MAX_PATH, m_pDmpFileName);
		TCHAR *pos = _tcsrchr(tmp, _T('\\'));
		if (pos)
		{
			pos++;
			_stprintf_s(m_pDmpFileName, MAX_PATH, _T("c:\\%s"), pos);
		}
		else
			_stprintf_s(m_pDmpFileName, MAX_PATH, _T("c:\\%s"), tmp);
	
		m_hDumpFile = CreateFile(m_pDmpFileName, GENERIC_WRITE, FILE_SHARE_READ,
									0, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH,	0);
	}

	int nError=0;
	
#ifdef TRY
	TRY
#endif
	{
		if (m_hDumpFile != INVALID_HANDLE_VALUE)
			writeMiniDump(pExceptionInfo);

		CloseHandle(m_hDumpFile);
		nError = 0;
	}
#ifdef TRY
	CATCH_ALL(e);
	{
		CloseHandle(m_hDumpFile);
	}
	END_CATCH_ALL
#endif
	

	if (nError)//happens some errors
	{
	}
	else  //the exception report has been saved
	{
		FreeLibrary(hDll);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	FreeLibrary(hDll);
	if (m_previousExceptionFilter)
		return m_previousExceptionFilter(pExceptionInfo);
	else
		return EXCEPTION_CONTINUE_SEARCH;

}


bool CExceptionReport::writeMiniDump(PEXCEPTION_POINTERS pExceptionInfo)
{
	// Write the minidump to the file
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = pExceptionInfo;
	eInfo.ClientPointers = FALSE;

	MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
	cbMiniDump.CallbackRoutine = 0;
	cbMiniDump.CallbackParam = 0;


	pMiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		m_hDumpFile,
		MiniDumpNormal,
		pExceptionInfo ? &eInfo : NULL,
		NULL,
		&cbMiniDump);

	// Close file
	CloseHandle(m_hDumpFile);
	return true;
}


void CExceptionReport::SuspendThreads()
{
	// Try to get OpenThread and Thread32* function from kernel32.dll, since it's not available on Win95/98
	typedef HANDLE (WINAPI *tOpenThread)	(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
	typedef BOOL (WINAPI *tThread32First)	(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
	typedef BOOL (WINAPI *tThread32Next)	(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
	typedef HANDLE (WINAPI *tCreateToolhelp32Snapshot)	(DWORD dwFlags, DWORD th32ProcessID);

	HMODULE hKernel32Dll = LoadLibrary(_T("kernel32.dll"));
	if (!hKernel32Dll)
		return;
	tOpenThread		pOpenThread		= (tOpenThread)		GetProcAddress(hKernel32Dll, "OpenThread");
	tThread32First	pThread32First	= (tThread32First)	GetProcAddress(hKernel32Dll, "Thread32First");
	tThread32Next	pThread32Next	= (tThread32Next)	GetProcAddress(hKernel32Dll, "Thread32Next");
	tCreateToolhelp32Snapshot pCreateToolhelp32Snapshot	= (tCreateToolhelp32Snapshot)	GetProcAddress(hKernel32Dll, "CreateToolhelp32Snapshot");
	if (!pOpenThread	||
		!pThread32First	||
		!pThread32Next	||
		!pCreateToolhelp32Snapshot)
	{
		CloseHandle(hKernel32Dll);
		return;
	}

	HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	// Get information about own process/thread
	DWORD ownProcessId = GetCurrentProcessId();
	DWORD ownThreadId = GetCurrentThreadId();

	// Enumerate threads
	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);
	BOOL bNext = pThread32First(hSnapshot, &entry);
	while (bNext)
	{
		if (entry.th32OwnerProcessID == ownProcessId &&
			entry.th32ThreadID != ownThreadId)
		{
			// Suspen threads of own process
			HANDLE hThread = pOpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID);
			if (hThread)
				SuspendThread(hThread);
		}
		bNext = pThread32Next(hSnapshot, &entry);
	}
	CloseHandle(hKernel32Dll);
}


const MSExecptionTString CExceptionReport::FormatCurrentTimeString()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	TCHAR szDateTime[1024];
	memset(szDateTime, 0 , sizeof(szDateTime));
	_stprintf_s(szDateTime , 1024, _T("%04d-%02d-%02d(%02d-%02d-%02d)"),
		st.wYear , st.wMonth , st.wDay , st.wHour , st.wMinute , st.wSecond);

	return MSExecptionTString(szDateTime);

}