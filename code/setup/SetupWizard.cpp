#include "StdAfx.h"
#include "SetupWizard.h"

#include <msapi/mswinapi.h>
#include <util/stdstringextend.h>
#include <util/registry.h>
#include <shlobj.h>
#include <mslog/msdkoutput.h>
#include <process/ParseCommand.h>
#include <xml/rapidtree_s.h>


class UnProgressCallback : public SevenZip::ProgressCallback
{
public:

	UnProgressCallback( CSetupWizard* pNotify)
	{
		m_totalSize = 0;
		m_pNotify = pNotify;
	}

	virtual bool OnFileCount(unsigned int fileCount)
	{ 
		fileCount = fileCount;
		return true; 

	};

	virtual bool OnFileItems(std::vector<std::wstring>&itemNames, std::vector<unsigned __int64>&itemSizes)
	{
		for ( std::vector<unsigned __int64>::iterator it = itemSizes.begin() ; it != itemSizes.end() ; it++ )
		{
			m_totalSize += *it;
		}
		return true; 
	};


	virtual void OnProgress(const std::wstring& filePath, unsigned __int64 bytesCompleted)
	{

	}

	virtual void OnRadio(unsigned __int64  inSize, unsigned __int64 outSize)
	{
		m_outSize = outSize;
		if( m_pNotify && m_pNotify->m_pNotify){
			m_pNotify->m_pNotify->OnExtractPercent( ((double)outSize / (double)m_totalSize) * 50  );
		}
	}

	unsigned __int64 m_totalSize;
	unsigned __int64 m_outSize;
	CSetupWizard* m_pNotify;

};

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
	if ( m_pNotify )
	{
		CString lpszPos = m_pNotify->GetProgramPosition();
		if (lpszPos)
		{
			m_strProgramPosition = lpszPos;
		}
	}

	USES_CONVERSION;
	outParam["path"] = W2A(m_strProgramPosition.GetBuffer());
	return TRUE;
}


BOOL CSetupWizard::LuaGetTempCheckOutPath(Json::Value& outParam)
{
	USES_CONVERSION;
	outParam["path"] = W2A(m_strCheckOutFilePath.GetBuffer());
	return TRUE;
}



HRESULT CSetupWizard::Run()
{
	return 0 ;
}

STDMETHODIMP CSetupWizard::Init(void*)
{
	CString strFileName;
	GetModuleFileName(NULL, strFileName.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strFileName.ReleaseBuffer();

	strFileName = strFileName.Mid(strFileName.ReverseFind('\\') + 1);
	strFileName = strFileName.Left(strFileName.ReverseFind('.'));

	std::vector<std::wstring> sp = strext::split(strFileName.GetBuffer(), L"_");
	
	for ( std::vector<std::wstring>::iterator it = sp.begin() ; it != sp.end() ; it++)
	{
		if ( wcsicmp(it->c_str(), _T("s")) == 0)
			m_bSilent = TRUE;
	}

	if ( sp.size() > 2)
	{
		m_dwOperID = _ttoi(sp[1].c_str());
	}


	

	//初始化DEBUG模式
	
	{
		INT nArg = 0;
		LPWSTR* pArgv = CommandLineToArgvW(GetCommandLine(), &nArg);

		for (INT nLoop = 0 ; nLoop < nArg ; nLoop++)
		{
			if (_tcsicmp(_T("/d"), pArgv[nLoop]) == 0)
			{
				m_bDebug = TRUE;
			}
			
			if ( _tcsicmp(_T("/s"), pArgv[nLoop]) == 0)
			{
				m_bSilent = TRUE;
			}
		}
		LocalFree(pArgv);
	}

	if (m_bDebug)
	{
		msapi::GetCurrentPath(m_strCheckOutFilePath.GetBufferSetLength(MAX_PATH), MAX_PATH);
		m_strCheckOutFilePath.ReleaseBuffer();
		m_strCheckOutFilePath += "\\";
	}else{
		srand(GetTickCount());
		GetTempPath(MAX_PATH, m_strCheckOutFilePath.GetBufferSetLength(MAX_PATH));
		m_strCheckOutFilePath.ReleaseBuffer();
		m_strCheckOutFilePath.AppendFormat(_T("%d_%d_%d\\"),rand(), rand(),rand());
		msapi::CreateDirectoryEx(m_strCheckOutFilePath);
	}
	
	


	//处理异常错误
	BOOL bRet = CheckOutFile();
	if ( !bRet )
	{
		if ( !m_bSilent )
		{
			MessageBox(NULL, _T("提取文件失败, 请重新下载安装包"), _T("错误"), MB_OK | MB_ICONERROR);
		}
		
		return E_FAIL;
	}

	bRet = ExtractConfigFile();
	if ( !bRet )
	{
		if ( !m_bSilent )
		{
			MessageBox(NULL, _T("提取配置文件失败, 请从官网下载最新安装包"), _T("错误"), MB_OK | MB_ICONERROR);
		}

		return E_FAIL;
	}

	bRet = LoadConfigFile();
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

	

	return S_OK;
}

STDMETHODIMP CSetupWizard::Uninit()
{
	

	if ( m_pNotify )
	{
		m_pNotify = INULL;
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
	
	if ( m_h7z )
	{
		LZMA_Close(m_h7z);
		m_h7z = NULL;
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
	
	//2、解压缩

	if( m_pNotify ){
		m_pNotify->OnStep(StepType_UnCompress);
	}

	BOOL bRet = UnCompress();
	if ( !bRet )
	{
		if ( m_pNotify )
		{
			m_pNotify->OnError(ErrorType_UnCompress);
		}

		return ErrorType_UnCompress;
	}


	//3、加载rot
	bRet = LoadRot();
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
	}else{
		if ( m_pNotify )
		{
			m_pNotify->OnFinesh();
		}
	}

	return 0;
}

BOOL CSetupWizard::CheckOutFile()
{
	if ( m_bDebug )
	{
		return TRUE;
	}
	//msapi::ExportResourceToFile()

	HRSRC hResource = ::FindResource(NULL, MAKEINTRESOURCE(IDR_PACKET), _T("PACKET"));
	if (!hResource)
		return FALSE;

	DWORD	dwSize  = ::SizeofResource(NULL, hResource);
	HGLOBAL hGlobal = ::LoadResource(NULL, hResource);

	if( !(hGlobal != NULL && dwSize) ) 
	{
		::FreeResource(hResource);
		return FALSE;
	}

	m_pMemPacket = new BYTE[dwSize];
	ZeroMemory(m_pMemPacket, dwSize);
	memcpy(m_pMemPacket, ::LockResource(hGlobal), dwSize);
	m_h7z = LZMA_OpenMem((void*)m_pMemPacket, dwSize);
	::FreeResource(hResource);
	return m_h7z ? TRUE : FALSE;;
	/*
	mspath::CPath::CreateDirectoryEx(lpszSave);
	HANDLE hSaveFile = CreateFile(lpszSave, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE == hSaveFile)
	{
		::FreeResource(hResource);
		return FALSE;
	}

	DWORD dwWriteSize = 0;
	BOOL bRet = WriteFile(hSaveFile, , dwSize, &dwWriteSize, NULL);

	::FreeResource(hResource);
	CloseHandle(hSaveFile);

	CString strModulePath;
	GetModuleFileName(NULL,strModulePath.GetBufferSetLength(MAX_PATH),MAX_PATH);
	strModulePath.ReleaseBuffer();

	UTIL::sentry<HANDLE, UTIL::handle_sentry> hHandle = CreateFile(strModulePath.GetBuffer(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	RASSERT(hHandle && hHandle != INVALID_HANDLE_VALUE, FALSE);
	*/
	

	/*
	DWORD dwPeSize = msapi::GetPESize(hHandle);
	DWORD dwFileSize = GetFileSize(hHandle, NULL);

	DWORD dwMemSize = dwFileSize - dwPeSize;
	if ( dwMemSize == 0)
	{
		return FALSE;
	}

	
	
	SetFilePointer(hHandle,dwPeSize,NULL,FILE_BEGIN);

	DWORD dwReadFromPe = 0;
	ReadFile(hHandle,m_pMemPacket,dwMemSize,&dwReadFromPe,NULL);
	
	*/
}

BOOL CSetupWizard::UnCompress()
{
	if ( m_bDebug )
	{
		return TRUE;
	}

	RASSERT(m_h7z , FALSE);
	

	CString strPacketActive = m_strCheckOutFilePath;
	strPacketActive.Append(_T("\\packet.7z"));

	msapi::ExportResourceToFile(NULL, IDR_PACKET, _T("PACKET"), strPacketActive);
	SevenZip::SevenZipExtractor decompress;
	UnProgressCallback call(this);
	if ( !decompress.SetArchivePath(strPacketActive.GetBuffer()) )
	{
		return FALSE;
	}
	if ( S_OK != decompress.ExtractArchive(m_strCheckOutFilePath.GetBuffer(), &call, NULL))
	{
		return FALSE;
	}
	/*
	//解压缩7Z包
	DWORD dw7zFileCount = LZMA_GetFileCount(m_h7z);
	for (DWORD dwIndex = 0 ; dwIndex < dw7zFileCount ; dwIndex++)
	{
		TCHAR szFileName[MAX_PATH] = { 0 };
		DWORD dwNameLen = LZMA_GetFileNameW(m_h7z, dwIndex, szFileName, MAX_PATH);
		
		
		CString sFile = m_strCheckOutFilePath; sFile.AppendFormat(_T("\\%s"), szFileName);
		DWORD dwLen = LZMA_ExtractW(m_h7z, dwIndex, sFile);

		if( m_pNotify ){
			m_pNotify->OnExtractPercent( ((double)dwIndex / (double)dw7zFileCount) * 50  );
		}
	}
	*/

	return TRUE;
}

BOOL CSetupWizard::LoadRot()
{
	CString strPath = m_strCheckOutFilePath;
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


	CString strmscomcfgPath = m_strCheckOutFilePath;
	strmscomcfgPath.Append(_T("\\mscomcfg.xml"));

	CString strRotPath = m_strCheckOutFilePath;
	strRotPath.Append(_T("\\setup.xml"));
	if (FAILED(pEnv->Init(strmscomcfgPath , /*lpszMsComXml*/ NULL , strRotPath ,/*lpszRotXml*/ NULL , NULL, m_strCheckOutFilePath)))
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

	CString strPath = m_strCheckOutFilePath;
	strPath += _T("\\script\\setup.lua");
	RFAILEDP(m_pLuaVM->DoFile(strPath),FALSE);

	Json::Value inParam;
	Json::Value outParam;
	if ( FAILED( this->CallLuaJsonFunction("Do", inParam, outParam) ))
	{
		return FALSE;
	}
	

	bool bRet = outParam["result"].asBool();
	if (m_pLuaVM)
	{
		m_pLuaVM->ClosetVM();
		m_pLuaVM = INULL;
	}

	return bRet ? TRUE : FALSE;
}

STDMETHODIMP_(DWORD) CSetupWizard::GetOperID()
{
	return m_dwOperID;
}

BOOL CSetupWizard::ExtractConfigFile()
{

	if ( m_bDebug )
	{
		return TRUE;
	}

	RASSERT(m_h7z , FALSE);

	BOOL bCheckOutXml = FALSE;
	BOOL bCheckOutSkin = FALSE;
	DWORD dw7zFileCount = LZMA_GetFileCount(m_h7z);
	for (DWORD dwIndex = 0 ; dwIndex < dw7zFileCount ; dwIndex++)
	{
		TCHAR szFileName[MAX_PATH] = { 0 };
		DWORD dwNameLen = LZMA_GetFileNameW(m_h7z, dwIndex, szFileName, MAX_PATH);
		if ( _tcsicmp(szFileName, _T("setup.xml")) == 0)
		{
			CString sFile = m_strCheckOutFilePath; sFile.AppendFormat(_T("\\%s"), szFileName);
			DWORD dwLen = LZMA_ExtractW(m_h7z, dwIndex, sFile);
			bCheckOutXml = TRUE;
		}
		else if (_tcsicmp(szFileName, _T("setup\\skin.zip")) == 0)
		{
			CString sFile = m_strCheckOutFilePath; sFile.AppendFormat(_T("\\%s"), szFileName);
			DWORD dwLen = LZMA_ExtractW(m_h7z, dwIndex, sFile);
			bCheckOutSkin = TRUE;
		}

		if (/* bCheckOutSkin &&*/ bCheckOutXml )
		{
			return TRUE;
		}
	}


	return FALSE;
}

BOOL CSetupWizard::LoadConfigFile()
{
	CString sFile = m_strCheckOutFilePath; sFile.AppendFormat(_T("\\%s"), _T("setup.xml"));
	
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

STDMETHODIMP_(LPCTSTR) CSetupWizard::GetTempCheckOutPath()
{
	return m_strCheckOutFilePath;
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

