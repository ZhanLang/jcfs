// updata.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "updata.h"

#include <mslog/logtool/logtool.h>
#include <crash/DumpFile.h>

#include <atlstr.h>

#include <mscom\mscominc.h>
#include <mslog/stdlog_dll.h>

#define GroupName _T("updata")


int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	DeclareDumpFile();
#ifdef DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//_CrtSetBreakAlloc(8764);
	//需要强制设置一下当前工作目录
	TCHAR lpszCurrpath[MAX_PATH] = {0};
	SetCurrentDirectory(msapi::GetCurrentPath(lpszCurrpath , MAX_PATH));


	CMSComLoader mscomLoader;

	if (!mscomLoader.LoadMSCom(_T(".\\Common File\\mscom\\envmgr.dll")))
	{
		ZM1_GrpDbgOutput(GroupName , _T("load envmgr.dll faild."));
		return 0;
	}

	UTIL::com_ptr<IMsEnvMgr> pEnv = INULL;

	if (FAILED(mscomLoader.CreateInstance(CLSID_MsEnv , NULL , NULL , re_uuidof(IMsEnvMgr) , (void**)&pEnv.m_p)))
	{
		ZM1_GrpDbgOutput(GroupName , _T("create env object faild."));
		return 0;
	}


	CString strProcName;
	GetModuleFileName(NULL, strProcName.GetBufferSetLength(MAX_PATH) ,MAX_PATH);
	strProcName.ReleaseBuffer();
	strProcName = strProcName.Mid(strProcName.ReverseFind('\\')+1);
	strProcName = strProcName.Left(strProcName.ReverseFind('.'));
	strProcName.Append(_T(".xml"));

	CString strRotPath;
	strRotPath.Format(_T("%s\\%s"), lpszCurrpath, strProcName.GetBuffer());
	CString strMsComPath;
	strMsComPath.Format(_T("%s\\%s"), lpszCurrpath, _T("mscomcfg.xml"));

	if (FAILED(pEnv->Init(strMsComPath , /*lpszMsComXml*/ NULL , strRotPath ,/*lpszRotXml*/ NULL , hInstance)))
	{
		//ZM1_GrpDbgOutput(GRP_NAME , _T("init env object faild.\r\n"));
		HRESULT hr =  pEnv->Uninit();

		pEnv = INULL;
		return hr;
	}
	else
	{
		if (FAILED( pEnv->Start()))
		{
			//ZM1_GrpDbgOutput(GRP_NAME,_T("start plugin faild.\r\n"));
		}

		if (FAILED(pEnv->Stop()))
		{
			//ZM1_GrpDbgOutput(GRP_NAME,_T("stop plugin faild.\r\n"));
		}
	}

	HRESULT hr = pEnv->Uninit();
	SAFE_RELEASE(pEnv)
		return hr;

}