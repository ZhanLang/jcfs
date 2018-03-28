// JCSrv.cpp : WinMain µÄÊµÏÖ


#include "stdafx.h"
#include "resource.h"
#include "JCSrv_i.h"
#include <stdio.h>

#include "ExceptionReport.h"
#include <msapi/mspath.h>
#include "NTService.h"

#include "SrvModule.h"


CSrvModule _AtlModule;
CExceptionReport ExceptionReport;
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
#ifdef DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	DbgActivate(_T("*"),9);
	//Sleep(9000);
	TCHAR lpszCurrpath[MAX_PATH] = {0};
	SetCurrentDirectory(msapi::GetCurrentPath(lpszCurrpath , MAX_PATH));

	_AtlModule.SetInstance(hInstance);
	return _AtlModule.WinMain(nShowCmd);
}

