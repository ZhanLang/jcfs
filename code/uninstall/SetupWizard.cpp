#include "StdAfx.h"
#include "SetupWizard.h"

#include <msapi/mswinapi.h>
#include <util/stdstringextend.h>
#include <util/registry.h>
#include <shlobj.h>
#include <mslog/msdkoutput.h>
#include <process/ParseCommand.h>

#include <xml/rapidtree_s.h>
#include <msapi/EnumFile.h>
#include <msapi/mstime.h>
#include "../../../msdk/msdk/include/msapi/msapp.h"
CSetupWizard::CSetupWizard(void)
{
	m_dwOperID = 0;
	m_bSilent = FALSE;
	m_h7z = NULL;
	m_bDebug = FALSE;
}


CSetupWizard::~CSetupWizard(void)
{

}

BOOL FileEnumFunction(LPCTSTR lpszPath, LPVOID param)
{
	std::vector<std::wstring>* sDelFiles = (std::vector<std::wstring>*)param;
	sDelFiles->push_back(lpszPath);
	return TRUE;
}

//lua extern function
BOOL CSetupWizard::LuaSetProgress(Json::Value inParam, Json::Value& outParam)
{
 	INT nPos = inParam["pos"].asInt();
	if (m_pNotify)
	{
		m_pNotify->OnSetProgress(nPos);
	}

 	outParam["result"] = true;
	return TRUE;
}

BOOL CSetupWizard::LuaGetControlCheckState( Json::Value inParam, Json::Value& outParam )
{
 	std::string ctrlName = inParam["ctrl"].asString();
 	outParam["state"] = true;

	if ( m_pNotify )
	{
		USES_CONVERSION;
		outParam["state"] = m_pNotify->GetControlCheckState(A2W(ctrlName.c_str())) ? true : false;
	}

	return TRUE;
}


BOOL CSetupWizard::LuaGetProgramPosition(Json::Value& outParam )
{

	USES_CONVERSION;
	outParam["path"] = W2A(m_strProgramPosition.GetBuffer());
	return TRUE;

	if ( m_pNotify )
	{
		CString lpszPos = m_pNotify->GetProgramPosition();
		if (lpszPos)
		{
			m_strProgramPosition = lpszPos;
		}
	}

	
}




HRESULT CSetupWizard::Run()
{
	return 0 ;
}

STDMETHODIMP CSetupWizard::Init(void*)
{
	CParseCommand praseCmd(TRUE);
	m_bSilent = praseCmd.GetArgInt(_T("s"));
	m_bDebug =  praseCmd.IsExist(_T("d"));
	LPCTSTR lpszPath = praseCmd.GetArg(_T("p"), NULL);
	
	//MessageBox(NULL, _T("tiaoshi"), _T("tiaoshi"), MB_OK);

	if ( !lpszPath )
	{
		//将文件拷贝到临时目录中
		TCHAR szCurPath[MAX_PATH] = { 0 };
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		praseCmd.SetArg(_T("p"), msapi::GetPathFilePath(szModulePath, szCurPath, MAX_PATH));

		
		CString strTempPath;
		GetTempPath(MAX_PATH, strTempPath.GetBufferSetLength(MAX_PATH));
		strTempPath.ReleaseBuffer();
		strTempPath.AppendFormat(_T("uninstall.exe"));

		CopyFile(szModulePath, strTempPath, FALSE);
		
		praseCmd.SetFilePath(strTempPath);
		msapi::Execute(praseCmd.GetFilePath(), praseCmd.GetCmdLine(TRUE).c_str(), FALSE, FALSE, 0);
		return E_FAIL;
	}
	else
		m_strProgramPosition = praseCmd.GetArg(_T("p"));

	BOOL bRet = LoadConfigFile();
	if ( !bRet )
	{
		if ( !m_bSilent )
		{
			MessageBox(NULL, _T("加载配置文件失败, 请从官网下载最新安装包"), _T("错误"), MB_OK | MB_ICONERROR);
		}

		return E_FAIL;
	}


	//初始化安装路径
	{

		msapi::CApp(GetProductName()).GetSetupPath( m_strProgramPosition.GetBufferSetLength(MAX_PATH), MAX_PATH);
		m_strProgramPosition.ReleaseBuffer();

		if ( m_strProgramPosition.IsEmpty() )
		{
			CString strRegPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
			strRegPath.Append(GetProductName());
			std::basic_string<TCHAR> sValue = CRegistry::GetString(HKEY_LOCAL_MACHINE, strRegPath, _T("InstallLocation"));
			if (sValue.length()){
				m_strProgramPosition = sValue.c_str();
			}else{
				if ( msapi::IsWindowsX64() ){
					::SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, SHGFP_TYPE_CURRENT, m_strProgramPosition.GetBufferSetLength(MAX_PATH));
					m_strProgramPosition.ReleaseBuffer();
				}else{
					::SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, m_strProgramPosition.GetBufferSetLength(MAX_PATH));
					m_strProgramPosition.ReleaseBuffer();
				}

				m_strProgramPosition.AppendFormat(_T("\\%s"), GetProductName());
			}
		}
	}

	

	return S_OK;
}


STDMETHODIMP CSetupWizard::Uninit()
{
	

	if ( m_pNotify )
	{
		m_pNotify = INULL;
	}

	
	

	if ( !m_bDebug )
	{
		//CFuncTime func(_T("CSetupWizard::Uninit"), _T("DeletePathFile"));
		//DeletePathFile(GetTempCheckOutPath());
	}
	
	

	return S_OK;
}

STDMETHODIMP CSetupWizard::Start()
{
	return S_OK;
}

STDMETHODIMP CSetupWizard::Stop()
{
	return S_OK;
}

STDMETHODIMP CSetupWizard::SetWizardNotify(ISetupWizardNotifty* pNotify)
{
	RASSERT(pNotify, E_FAIL);
	m_pNotify = pNotify;
	return S_OK;
}

STDMETHODIMP_(BOOL) CSetupWizard::GetSilent()
{
	return m_bSilent;
}



STDMETHODIMP_(INT) CSetupWizard::DoSetup()
{
		
	//3、加载rot
	BOOL bRet = LoadRot();
	if ( !bRet )
	{
		if ( m_pNotify )
		{
			m_pNotify->OnError(ErrorType_LoadRot);
		}

		return ErrorType_LoadRot;
	}

	if ( m_pNotify )
	{
		m_pNotify->OnStep(StepType_DoScript);
	}
	//4、执行lua脚本
	bRet = DoSetupLua();
	if ( !bRet )
	{
		if ( m_pNotify )
		{
			m_pNotify->OnError(ErrorType_DoScript);
		}

		return ErrorType_DoScript;
	}

	if ( m_pEnv )
	{
		m_pEnv->Stop();
		m_pEnv->Uninit();
		m_pEnv = INULL;
	}

	if (m_mscomLoader.IsLoaded())
	{
		m_mscomLoader.UnloadMSCom();
	}

	//删除文件
	std::vector<std::wstring> sDelFiles;
	msapi::CEnumFile enumFile(FileEnumFunction, &sDelFiles);
	enumFile.EnumFile(m_strProgramPosition, _T("*.*"));

	for ( DWORD dwLoop = 0 ; dwLoop < sDelFiles.size() ; dwLoop++)
	{
		std::wstring sPath = sDelFiles[dwLoop];
		if ( !DeleteFile(sPath.c_str()) )
		{
			TCHAR szTime[MAX_PATH] = { 0 };
			msapi::GetCurrentTimeStr(szTime, MAX_PATH);
			sPath.append(szTime);
			MoveFileEx(sPath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
		if( m_pNotify )
			m_pNotify->OnSetProgress( ((double)dwLoop / (double)sDelFiles.size()) * 50 + 50 );
	}

	if( m_pNotify )
		m_pNotify->OnSetProgress( 100 );

	msapi::DeleteFileEx(m_strProgramPosition);
	msapi::DeletePathFile(m_strProgramPosition);

	if ( m_pNotify )
		m_pNotify->OnFinesh();
	
	return 0;
}

BOOL CSetupWizard::CheckOutFile()
{
	if ( m_bDebug )
	{
		return TRUE;
	}
	
	return TRUE;
}

BOOL CSetupWizard::UnCompress()
{
	if ( m_bDebug )
	{
		return TRUE;
	}

	
	return TRUE;
}

BOOL CSetupWizard::LoadRot()
{
	CString strPath = m_strProgramPosition;
	strPath.AppendFormat(_T("\\Common File\\mscom\\envmgr.dll"));
	if (!m_mscomLoader.LoadMSCom(strPath))
	{
		return FALSE;
	}

	UTIL::com_ptr<IMsEnvMgr> pEnv = INULL;
	if (FAILED(m_mscomLoader.CreateInstance(CLSID_MsEnv , NULL , NULL , re_uuidof(IMsEnvMgr) , (void**)&pEnv.m_p)))
	{
		return 0;
	}

	if (!pEnv)
	{
		return FALSE;
	}


	CString strmscomcfgPath = m_strProgramPosition;
	strmscomcfgPath.Append(_T("\\mscomcfg.xml"));

	CString strRotPath = m_strProgramPosition;
	strRotPath.Append(_T("\\uninstall.xml"));
	if (FAILED(pEnv->Init(strmscomcfgPath , /*lpszMsComXml*/ NULL , strRotPath ,/*lpszRotXml*/ NULL , NULL, m_strProgramPosition)))
	{
		HRESULT hr =  pEnv->Uninit();
		pEnv = INULL;
		return hr;
	}

	if ( FAILED( pEnv->Start() ))
	{
		pEnv->Uninit();
		pEnv->Stop();
		pEnv = INULL;
		return E_FAIL;
	}
	

	m_pEnv = pEnv;

	return TRUE;
}

BOOL CSetupWizard::DoSetupLua()
{
	UTIL::com_ptr<IMsEnv> pEnv = m_pEnv;

	UTIL::com_ptr<IPropertyStr> envPropertyStr;
	pEnv->GetEnvParam((IMSBase**)&envPropertyStr.m_p);

	//设置初始化环境变量
	CPropStrSet propstrSet(envPropertyStr);
	propstrSet["silent"] = GetSilent();
	propstrSet["new_operid"] = GetOperID();


	UTIL::com_ptr<IMscomRunningObjectTable> pRot;
	m_pEnv->GetRunningObjectTable(&pRot.m_p);

	pRot->CreateInstance(CLSID_LuaVM, NULL, re_uuidof(ILuaVM), (void**)&m_pLuaVM);
	m_pLuaVM->Register(this);

	RFAILEDP(m_pLuaVM->OpenVM(), FALSE);

	CString strPath = m_strProgramPosition;
	strPath += _T("\\script\\uninst.lua");
	RFAILEDP(m_pLuaVM->DoFile(strPath),FALSE);

	Json::Value inParam;
	Json::Value outParam;
	this->CallLuaJsonFunction("Do", inParam, outParam);

	if (m_pLuaVM)
	{
		m_pLuaVM->ClosetVM();
		m_pLuaVM = INULL;
	}

	return TRUE;
}

STDMETHODIMP_(DWORD) CSetupWizard::GetOperID()
{
	return m_dwOperID;
}

BOOL CSetupWizard::ExtractConfigFile()
{

	return FALSE;
}

BOOL CSetupWizard::LoadConfigFile()
{
	CString sFile = m_strProgramPosition; sFile.AppendFormat(_T("\\%s"), _T("uninstall.xml"));

	CRapidTreeS xml;
	RASSERT(xml.Load(sFile),FALSE);

	HXMLTREEITEM hRoot = xml.GetRoot();
	m_strProductName = xml.GetText(xml.LocalItem(hRoot, _T("mscomcfg\\envinfo\\productname")));
	m_strFriendProdName = xml.GetText( xml.LocalItem(hRoot,_T("mscomcfg\\envinfo\\friendprodname")));
	m_strVersion = xml.GetText( xml.LocalItem(hRoot,_T("mscomcfg\\envinfo\\version")));


	if ( m_strVersion.GetLength() && m_strFriendProdName.GetLength() && m_strVersion.GetLength())
	{
		return TRUE;
	}

	return FALSE;
}


STDMETHODIMP_(LPCTSTR) CSetupWizard::GetProductName()
{
	return m_strProductName;
}

STDMETHODIMP_(LPCTSTR) CSetupWizard::GetFriendProdName()
{
	return m_strFriendProdName;
}

STDMETHODIMP_(LPCTSTR) CSetupWizard::GetProuctId()
{
	return m_strProuctId;
}

STDMETHODIMP_(LPCTSTR) CSetupWizard::GetVersion()
{
	return m_strVersion;
}

STDMETHODIMP_(LPCTSTR) CSetupWizard::GetMutex()
{
	return m_strVersion;
}

STDMETHODIMP_(LPCTSTR) CSetupWizard::GetProgramPosition()
{
	return m_strProgramPosition;
}

STDMETHODIMP_(BOOL) CSetupWizard::IsDebug()
{
	return m_bDebug;
}

BOOL CSetupWizard::LuaGetSilent(Json::Value& outParam)
{
	outParam["silent"] = m_bSilent ? true : false;
	return TRUE;
}

