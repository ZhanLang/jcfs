// JCmd.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JCmd.h"

#include <process/ParseCommand.h>
#include <mslog/stdlog_dll.h>
#include <msapi/mssrv.h>
#include "ServiceOpt.h"
#include <shellapi.h>
#include <msapi/mswinapi.h>
#include <vistafunc/vistafunc.h>
using namespace msdk;;

#define GroupName _T("JCmd")

bool DllRegisterServer(LPCTSTR lpszDllPath, bool bInst)
{
	if ( !PathFileExists(lpszDllPath) )
	{
		GrpError(GroupName, MsgLevel_Error, _T("DllRegisterServer(%s)"), _T("文件不存在"));
		return false;
	}

	GrpMsg(GroupName, MsgLevel_Msg, _T("DllRegisterServer (%s,%d)"), lpszDllPath, bInst);

	BOOL b64PeFile = msapi::Is64PeFile(lpszDllPath);
	if ( b64PeFile )
		GrpError(GroupName, MsgLevel_Error, _T("DllRegisterServer(%s)"), _T("64位DLL"));


	BOOL bWin64 = msapi::IsWindowsX64();

	//暂时不考虑自身是否64位的情况

	LPVOID pWow64FsRedirection = NULL;
	if ( bWin64 && b64PeFile)//需要重定向
		vistafunc::DisableWow64FsRedirection(&pWow64FsRedirection);

	CString strCmd; strCmd.Format(bInst ? _T("%s \"%s\" /s") : _T("%s \"%s\" /s /u"),_T("C:\\Windows\\System32\\regsvr32.exe"), lpszDllPath);

	DWORD dwRet = msapi::Execute(_T("C:\\Windows\\System32\\regsvr32.exe"), strCmd, TRUE, FALSE, -1);
	if ( dwRet )
		GrpError(GroupName, MsgLevel_Error, _T("DllRegisterServer(%s) 失败[%d]"), lpszDllPath, dwRet);

	if ( pWow64FsRedirection )
		vistafunc::RevertWow64FsRedirection(pWow64FsRedirection);

	return dwRet == 0;
}

BOOL RegisterJCfsClient()
{
	CString strPath;
	msapi::GetCurrentPath(strPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strPath.ReleaseBuffer();
	strPath.Append(_T("\\common file\\jcfs\\JCfsClient.dll"));

	return DllRegisterServer(strPath, true) ? TRUE : FALSE;
}

// startsrv=jcsrv
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	CParseCommand ParseCmd(TRUE);
	if ( ParseCmd.IsExist(_T("install_jcsrv")))
	{
		if ( !RegisterJCfsClient() )
			return FALSE;
		

		if ( !msapi::IsSrvExisted(_T("JCSrv")) )
		{
			//安装服务
			CString strJCfs;
			msapi::GetCurrentPath(strJCfs.GetBufferSetLength(MAX_PATH), MAX_PATH);
			strJCfs.ReleaseBuffer();
			strJCfs.Append(_T("\\JCSrv.exe"));

			SHELLEXECUTEINFO shellInfo = { 0 };
			shellInfo.cbSize = sizeof( shellInfo );
			shellInfo.lpVerb = _T("runas");
			shellInfo.hwnd = NULL;
			shellInfo.lpFile =(LPCWSTR)strJCfs;
			shellInfo.lpParameters = _T("/Service");
			shellInfo.nShow = SW_SHOWNORMAL;
			shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
			if ( ShellExecuteEx( &shellInfo ) )
				WaitForSingleObject(shellInfo.hProcess, -1);
		}

		//只有在停止的时候去处理这件事情，要不然会导致服务启动失败
		if ( !msapi::IsSrvStoped(_T("JCSrv")) )
		{
			CServiceOpt SrvOp(_T("JCSrv"));
			if ( SrvOp.Init() == S_OK )
			{
				if ( SrvOp.Start(FALSE) == S_OK)
					return TRUE;
			}
		}

		return FALSE;
	}

	return 0;
}