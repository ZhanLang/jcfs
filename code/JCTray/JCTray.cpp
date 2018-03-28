// VCDRom.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <mscom/mscominc.h>
#include "ExceptionReport.h"
#include <IDeskTray.h>
#include <shellapi.h>
#include <process/ParseCommand.h>
#include <msapi/mssrv.h>

CExceptionReport ExceptionReport;
using namespace mscom;
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	
#ifdef DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//_CrtSetBreakAlloc(239);
	//需要强制设置一下当前工作目录
	TCHAR lpszCurrpath[MAX_PATH] = {0};
	SetCurrentDirectory(msapi::GetCurrentPath(lpszCurrpath , MAX_PATH));


	//如果服务还是没有安装，提示用户退出
	if ( !( msapi::IsSrvExisted(_T("JCSrv")) && msapi::IsSrvRunning(_T("JCSrv"))) )
	{
		//安装服务
		CString strJCmd;
		msapi::GetCurrentPath(strJCmd.GetBufferSetLength(MAX_PATH), MAX_PATH);
		strJCmd.ReleaseBuffer();
		strJCmd.Append(_T("\\JCmd.exe"));

		CString strCmd;
		CParseCommand ParseCmd(FALSE);
		ParseCmd.SetArg(_T("install_jcsrv"), _T("1"));
		strCmd = ParseCmd.GetCmdLine().c_str();

		SHELLEXECUTEINFO shellInfo = { 0 };
		shellInfo.cbSize = sizeof( shellInfo );
		shellInfo.lpVerb = _T("runas");
		shellInfo.hwnd = NULL;
		shellInfo.lpFile =(LPCWSTR)strJCmd;

		shellInfo.lpParameters = strCmd;
		shellInfo.nShow = SW_SHOWNORMAL;
		shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
		if ( ShellExecuteEx( &shellInfo ) )
			WaitForSingleObject(shellInfo.hProcess, -1);
	}

	//再次判断，如果还是不行提示用户并退出
	if ( !( msapi::IsSrvExisted(_T("JCSrv")) && msapi::IsSrvRunning(_T("JCSrv"))) )
	{
		MessageBox(NULL, _T("启动基础服务失败"), _T("错误"), MB_OK);
		return 0;
	}

	CMSComLoader mscomLoader;

	if (!mscomLoader.LoadMSCom(_T(".\\Common File\\mscom\\envmgr.dll")))
	{
		//GrpError(GroupName, MsgLevel_Error, _T("load envmgr.dll faild."));
		return 0;
	}

	UTIL::com_ptr<IMsEnvMgr> pEnv = INULL;

	if (FAILED(mscomLoader.CreateInstance(CLSID_MsEnv , NULL , NULL , re_uuidof(IMsEnvMgr) , (void**)&pEnv.m_p)))
	{
		//GrpError(GroupName, MsgLevel_Error, _T("create env object faild."));
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