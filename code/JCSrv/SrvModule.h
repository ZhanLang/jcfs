#pragma once
#include <msapi/mspath.h>
#include "NTService.h"
#include "JCSrv_i.h"

class CSrvModule : 
		public ATL::CAtlServiceModuleT< CSrvModule, IDS_SERVICENAME >
	{
	public :

		typedef ATL::CAtlServiceModuleT< CSrvModule, IDS_SERVICENAME > CAtlSrv;
		DECLARE_LIBID(LIBID_JCSrvLib)
		DECLARE_REGISTRY_APPID_RESOURCEID(IDR_JCSRV, "{8A943A86-8398-41C5-ACF3-22A3B2FBAAB7}")

	public:
		CSrvModule()
		{
			m_status.dwControlsAccepted = 
				SERVICE_ACCEPT_STOP		   | 
			//	SERVICE_ACCEPT_SHUTDOWN      | 
				SERVICE_ACCEPT_SESSIONCHANGE |
				//SERVICE_ACCEPT_NETBINDCHANGE |
			//	SERVICE_ACCEPT_PARAMCHANGE   |
				SERVICE_ACCEPT_PAUSE_CONTINUE//|
				//SERVICE_ACCEPT_PRESHUTDOWN  
				;


		}
		HRESULT InitializeSecurity() throw()
		{
			GrpMsg(GroupName, MsgLevel_Msg, _T("InitializeSecurity"));

	//	return S_OK;
			return CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL );
		}

		

		

		HRESULT Run(_In_ int nShowCmd = SW_HIDE) throw()
		{
			GrpMsg(GroupName, MsgLevel_Msg, _T("Run"));

			TCHAR lpszCurrpath[MAX_PATH] = {0};
			msapi::GetCurrentPath(lpszCurrpath , MAX_PATH);

			CString strEnvPath; strEnvPath.Format(_T("%s\\Common File\\mscom\\envmgr.dll"), lpszCurrpath);
			CMSComLoader mscomLoader;

			if ( !mscomLoader.LoadMSCom(strEnvPath) )
			{
				GrpError(GroupName, MsgLevel_Error, _T("load envmgr.dll faild."));
				return E_FAIL;
			}


			if (FAILED(mscomLoader.CreateInstance(CLSID_MsEnv , NULL , NULL , re_uuidof(IMsEnvMgr) , (void**)&m_pEnvMgr.m_p)))
			{
				GrpError(GroupName, MsgLevel_Error, _T("create env object faild."));
				return 0;
			}

			{	//因为是外部服务所以得先设置服务
				UTIL::com_ptr<IEvnSrv> pSrv = m_pEnvMgr;
				if ( pSrv )
				{
					GrpMsg(GroupName, MsgLevel_Msg, _T("SetServiceStatusHandle:%d"), m_hServiceStatus);
					pSrv->SetServiceStatusHandle(m_hServiceStatus);
				}
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

			if (FAILED(m_pEnvMgr->Init(strMsComPath , /*lpszMsComXml*/ NULL , strRotPath ,/*lpszRotXml*/ NULL , m_hInstance)))
			{
				GrpError(GroupName, MsgLevel_Error, _T("init env object faild"));
				HRESULT hr =  m_pEnvMgr->Uninit();
				m_pEnvMgr = INULL;
				return hr;
			}
			else
			{
				if (FAILED( m_pEnvMgr->Start()))
				{
					GrpError(GroupName, MsgLevel_Error, _T("start plugin faild"));
				}

				SetServiceStatus(SERVICE_START_PENDING);
				HRESULT hR = __super::Run(nShowCmd);
				if (FAILED(m_pEnvMgr->Stop()))
				{
					//ZM1_GrpDbgOutput(GRP_NAME,_T("stop plugin faild.\r\n"));
				}
			}

			HRESULT hr = m_pEnvMgr->Uninit();
			SAFE_RELEASE(m_pEnvMgr)
			return hr;
		}

		inline HRESULT RegisterAppId(_In_ bool bService = false) throw()
		{
			if (!Uninstall())
				return E_FAIL;

			HRESULT hr = __super::UpdateRegistryAppId(TRUE);
			if (FAILED(hr))
				return hr;

			CRegKey keyAppID;
			LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
			if (lRes != ERROR_SUCCESS)
				return AtlHresultFromWin32(lRes);

			CRegKey key;

			lRes = key.Create(keyAppID, GetAppIdT());
			if (lRes != ERROR_SUCCESS)
				return AtlHresultFromWin32(lRes);

			key.DeleteValue(_T("LocalService"));

			if (!bService)
				return S_OK;

			key.SetStringValue(_T("LocalService"), m_szServiceName);

			// Create service
			if (!Install())
				return E_FAIL;
			return S_OK;
		}


		BOOL Install() throw()
		{
			if (IsInstalled())
				return TRUE;

			// Get the executable file path
			TCHAR szFilePath[MAX_PATH + _ATL_QUOTES_SPACE];
			DWORD dwFLen = ::GetModuleFileName(NULL, szFilePath + 1, MAX_PATH);
			if( dwFLen == 0 || dwFLen == MAX_PATH )
				return FALSE;

			// Quote the FilePath before calling CreateService
			szFilePath[0] = _T('\"');
			szFilePath[dwFLen + 1] = _T('\"');
			szFilePath[dwFLen + 2] = 0;

			SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (hSCM == NULL)
			{
				TCHAR szBuf[1024];
				if (AtlLoadString(ATL_SERVICE_MANAGER_OPEN_ERROR, szBuf, 1024) == 0)
#ifdef UNICODE
					Checked::wcscpy_s(szBuf, _countof(szBuf), _T("Could not open Service Manager"));
#else
					Checked::strcpy_s(szBuf, _countof(szBuf), _T("Could not open Service Manager"));
#endif
				MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
				return FALSE;
			}

			SC_HANDLE hService = ::CreateService(
				hSCM, m_szServiceName, m_szServiceName,
				SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
				szFilePath, NULL, NULL, _T("RPCSS\0"), NULL, NULL);

			if (hService == NULL)
			{
				::CloseServiceHandle(hSCM);
				TCHAR szBuf[1024];
				if (AtlLoadString(ATL_SERVICE_START_ERROR, szBuf, 1024) == 0)
#ifdef UNICODE
					Checked::wcscpy_s(szBuf, _countof(szBuf), _T("Could not start service"));
#else
					Checked::strcpy_s(szBuf, _countof(szBuf), _T("Could not start service"));
#endif
				MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
				return FALSE;
			}

			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCM);
			return TRUE;
		}

		VOID SetInstance(HINSTANCE h)
		{
			m_hInstance = h;
		}

	public:
		HINSTANCE m_hInstance;
		UTIL::com_ptr<IMsEnvMgr> m_pEnvMgr;
};

