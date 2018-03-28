// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define  _WTL_NO_CSTRING	
#define _WTL_NO_WTYPES

#include <atlbase.h>
#include <atlstr.h>	
#include <atlapp.h>



#include <atlimage.h>

using namespace WTL;






#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif


#include "resource.h"


#include <UIlib.h>
using namespace DuiLib;

#include "mscom/mscominc.h"
#include "mslog/stdlog_dll.h"

#define GroupName _T("mainui")




// TODO: reference additional headers your program requires here


static CString FormatFileTime(FILETIME fileTime)
{
	SYSTEMTIME sysTime = {0};
	FileTimeToSystemTime(&fileTime,&sysTime);
	TCHAR szTime[MAX_PATH] = {0};
	_stprintf_s(szTime, _countof(szTime), _T("%04d/%02d/%02d %02d:%02d"),
		sysTime.wYear, 
		sysTime.wMonth, 
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute
		);
	return szTime;
}

static CString FormatFileSize(DWORDLONG dwlFileSize)
{
	CString strRet;
	if (!dwlFileSize)
	{
		return strRet;
	}

	double lSize = (double)dwlFileSize;
	WCHAR szInfo[MAX_PATH] = {0};
	if(lSize < 1024L) //B
	{
		swprintf_s(szInfo,L"%d(B)",MAX_PATH,(DWORD)lSize);
	}
	else if (lSize< 1048576) //K
	{
		swprintf_s(szInfo,MAX_PATH,L"%0.2f(K)",lSize/1024);
	}
	else if (lSize < 1073741824) //M
	{
		swprintf_s(szInfo,MAX_PATH,L"%0.2f(M)",lSize/1048576);
	}
	else
	{
		swprintf_s(szInfo,MAX_PATH,L"%0.2f(G)",lSize/1073741824);
	}

	strRet = szInfo;
	return strRet;
}

#include <shlobj.h>

static bool WriteAsciiStringToClipboard(const wchar_t * sClipdata, HWND hOwningWnd)
{
	if (OpenClipboard(hOwningWnd))
	{
		EmptyClipboard();
		HGLOBAL hClipboardData;
		size_t sLen = _tcslen(sClipdata);
		hClipboardData = GlobalAlloc(GMEM_DDESHARE, (sLen+1)*sizeof(wchar_t));
		if (hClipboardData)
		{
			wchar_t * pchData;
			pchData = (wchar_t*)GlobalLock(hClipboardData);
			if (pchData)
			{
				_tcscpy_s(pchData, sLen+1, sClipdata);
				if (GlobalUnlock(hClipboardData))
				{
					if (SetClipboardData(CF_UNICODETEXT, hClipboardData) == NULL)
					{
						CloseClipboard();
						return true;
					}
				}
				else
				{
					CloseClipboard();
					return false;
				}
			}
			else
			{
				CloseClipboard();
				return false;
			}
		}
		else
		{
			CloseClipboard();
			return false;
		}
		CloseClipboard();
		return false;
	}
	return false;
}

/*
static int CopyFileToClipboard(LPCTSTR szFileName)
{
	UINT uDropEffect;
	HGLOBAL hGblEffect;
	LPDWORD lpdDropEffect;
	DROPFILES stDrop;

	HGLOBAL hGblFiles;
	LPSTR lpData;

	//与剪贴板交互的数据格式并非标准剪贴板格式，因此需要注册
	uDropEffect = RegisterClipboardFormat(_T("Preferred DropEffect"));

	//为剪贴板数据分配空间
	hGblEffect = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE,sizeof(DWORD));
	if (hGblEffect == NULL)
	{
		return 0;
	}

	lpdDropEffect = (LPDWORD)GlobalLock(hGblEffect);
	if (lpdDropEffect == NULL)
	{
		GlobalFree(hGblEffect);
		return 0;
	}


	//设置操作的模式
	*lpdDropEffect = DROPEFFECT_COPY;//复制; 剪贴则用DROPEFFECT_MOVE
	GlobalUnlock(hGblEffect);

	//剪贴板数据需要一个这个结构
	stDrop.pFiles = sizeof(DROPFILES);//文件列表相对于结构的偏移量
	stDrop.pt.x = 0;
	stDrop.pt.y = 0;
	stDrop.fNC = FALSE;
	stDrop.fWide = sizeof(szFileName[0]) != 1 ? TRUE : FALSE;//宽字符设置

	//分配数据空间，并且预留文件名的空间
	//文件列表为两个'\0'结尾，因此需要多分配2个字节
	INT nLen = sizeof(DROPFILES)+ _tcslen(szFileName) * sizeof(TCHAR)+ 2*sizeof(TCHAR) + 20;
	hGblFiles = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE,nLen);

	if (!hGblFiles)
	{
		GlobalFree(hGblEffect);
		return 0;
	}

	lpData = (LPSTR)GlobalLock(hGblFiles);
	if (lpData == NULL)
	{
		GlobalFree(hGblEffect);
		GlobalFree(hGblFiles);
	}

	memcpy(lpData, &stDrop, sizeof(DROPFILES));
	_tcscpy_s((LPTSTR)(lpData + sizeof(DROPFILES)), nLen, szFileName);

	GlobalUnlock(hGblFiles);

	//普通的剪贴板操作
	if (OpenClipboard(NULL))
	{
		if (EmptyClipboard())
		{
			//设置剪贴板文件信息
			SetClipboardData(CF_HDROP,hGblFiles);
			//设置剪贴板操作信息
			SetClipboardData(uDropEffect,hGblEffect);
		}
		CloseClipboard();
	}

	//GlobalFree(hGblEffect);
	//GlobalFree(hGblFiles);

	return 1;
}
*/

static int CopyFileToClipboard(const char szFileName[], DWORD dwDropEffect = DROPEFFECT_COPY)
{
	UINT uDropEffect;
	HGLOBAL hGblEffect;
	LPDWORD lpdDropEffect;
	DROPFILES stDrop;

	HGLOBAL hGblFiles;
	LPSTR lpData;

	//与剪贴板交互的数据格式并非标准剪贴板格式，因此需要注册
	uDropEffect = RegisterClipboardFormatA("Preferred DropEffect");
	//为剪贴板数据分配空间
	hGblEffect = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE,sizeof(DWORD));
	lpdDropEffect = (LPDWORD)GlobalLock(hGblEffect);
	//设置操作的模式
	*lpdDropEffect = dwDropEffect;//复制; 剪贴则用DROPEFFECT_MOVE
	GlobalUnlock(hGblEffect);

	//剪贴板数据需要一个这个结构
	stDrop.pFiles = sizeof(DROPFILES);//文件列表相对于结构的偏移量
	stDrop.pt.x = 0;
	stDrop.pt.y = 0;
	stDrop.fNC = FALSE;
	stDrop.fWide = FALSE;//宽字符设置

	//分配数据空间，并且预留文件名的空间
	//文件列表为两个'\0'结尾，因此需要多分配2个字节
	hGblFiles = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE,\
		sizeof(DROPFILES)+strlen(szFileName)+2);

	lpData = (LPSTR)GlobalLock(hGblFiles);
	memcpy(lpData,&stDrop,sizeof(DROPFILES));
	strcpy(lpData+sizeof(DROPFILES),szFileName);
	GlobalUnlock(hGblFiles);

	//普通的剪贴板操作
	OpenClipboard(NULL);
	EmptyClipboard();
	//设置剪贴板文件信息
	SetClipboardData(CF_HDROP,hGblFiles);
	//设置剪贴板操作信息
	SetClipboardData(uDropEffect,hGblEffect);
	CloseClipboard();

	return 1;
}

static BOOL IsWindowsX64()
{
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	SYSTEM_INFO si = {0};
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&si);
	else 
		GetSystemInfo(&si); 

	if (PROCESSOR_ARCHITECTURE_IA64 == si.wProcessorArchitecture ||  
		PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture)
	{
		return TRUE;
	}

	return FALSE;
}

#define DEFULT_IO_OPERATION_BYTES	1024
static DWORD Execute(LPCTSTR appfile, LPCTSTR commandline, bool waitforexit, bool showconcle, int waittime)
{
	RASSERT(appfile,-1);

	TCHAR _errinfo[DEFULT_IO_OPERATION_BYTES * 2 + 1] = {0};

	_errinfo[0] = NULL;
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	sa.bInheritHandle		= TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength				= sizeof(sa);

	HANDLE hread  = NULL,hwrite = NULL,herrin = NULL,herrout= NULL;

	DWORD  result = -1;

	do {
		if (waitforexit
			&& ::CreatePipe(&hread, &hwrite, &sa,	DEFULT_IO_OPERATION_BYTES * 200)
			&& ::CreatePipe(&herrin, &herrout, &sa, DEFULT_IO_OPERATION_BYTES * 200)
			&& ::SetHandleInformation(herrin, HANDLE_FLAG_INHERIT, 0)
			)
		{
			si.dwFlags	  = STARTF_USESTDHANDLES;
			si.hStdError  = herrout;
			si.hStdOutput = hwrite;
			si.hStdInput  = hread;
		}

		DWORD createflag = ::GetPriorityClass(::GetCurrentProcess());
		if (!showconcle) createflag |= CREATE_NO_WINDOW;
		BOOL err = ::CreateProcess(appfile, (LPTSTR)commandline,
			NULL, NULL, TRUE, 
			createflag, NULL, NULL,
			&si, &pi
			);
		CloseHandle(hwrite);
		CloseHandle(herrout);
		if (!err) {
			TCHAR szErr[MAX_PATH] = {0};
			//GrpError(GRP_NAME,MsgLevel_Error, _T("执行进程失败了,file:[%s];cmd:[%s],err:[%s]"),appfile,commandline,GetLastErrorText(szErr,MAX_PATH));
			break;
		}


		if (!waitforexit) {
			result = 0;
			break;
		}

		DWORD r = ::WaitForSingleObject(pi.hProcess, waittime);
		if (r != WAIT_OBJECT_0) {
			::TerminateProcess(pi.hProcess, -1);
			//GrpError(GRP_NAME, MsgLevel_Error, _T("Execute TerminateProcess(%s), as %d"), appfile?appfile:_T("null"), r);
		}
		::GetExitCodeProcess(pi.hProcess, &result);

		DWORD outlen = 0,
			errlen = 0;
		if (hread != NULL && herrin != NULL) {
			//::ReadFile(hread,  _errinfo,		  DEFULT_IO_OPERATION_BYTES, &outlen, NULL);
			//::ReadFile(herrin, _errinfo + outlen, DEFULT_IO_OPERATION_BYTES, &errlen, NULL);
		}
		_errinfo[errlen + outlen] = NULL;

	} while (0);

	CloseHandle(hread);
	CloseHandle(herrin);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	_errinfo[500] = NULL;

	//GrpError(GRP_NAME, MsgLevel_Error, _T("进程执行结果:[%x]"), result);
	return result;
}
