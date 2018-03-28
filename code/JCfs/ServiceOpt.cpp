#include "StdAfx.h"
#include "ServiceOpt.h"


CServiceOpt::CServiceOpt(void)
:m_hThread(NULL),m_hThreadExit(NULL),m_hNotifyThreadWork(NULL),m_pWorkParam(NULL)
,m_dwWaitTimeOut(DEFAULT_WAIT_TIMEOUT),m_pCallback(NULL)
{
	ZeroMemory(m_strSrvName, sizeof(m_strSrvName));
}

CServiceOpt::CServiceOpt(LPCTSTR lpName)
:m_hThread(NULL),m_hThreadExit(NULL),m_hNotifyThreadWork(NULL),m_pWorkParam(NULL)
,m_dwWaitTimeOut(DEFAULT_WAIT_TIMEOUT),m_pCallback(NULL)
{
	SetServiceName(lpName);
}

CServiceOpt::~CServiceOpt(void)
{
	Uninit();
}

void  CServiceOpt::SetServiceName(LPCTSTR lpName)
{
	if(lpName)
	{
		_tcscpy_s(m_strSrvName, MAX_PATH, lpName);
	}
}

LPCTSTR  CServiceOpt::GetServiceName()
{
	return m_strSrvName;
}

HRESULT CServiceOpt::Init(IServiceOptCallback* pCallback/* = NULL*/, DWORD dwWaitTimeOut/* = DEFAULT_WAIT_TIMEOUT*/)
{
	m_dwWaitTimeOut = dwWaitTimeOut;
	m_pCallback = pCallback;

	// Get a handle to the SCM database. 
	m_schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == m_schSCManager) 
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return E_FAIL;
	}

	return S_OK;
}

void  CServiceOpt::Uninit()
{
	if(m_hThread && m_hThreadExit)
	{
		SetEvent(m_hThreadExit);
		WaitForSingleObject(m_hThread, INFINITE);
	}

	if (m_hThread)
	{
		CloseHandle(m_hThread);
	}

	if(m_hThreadExit)
	{
		CloseHandle(m_hThreadExit);
		m_hThreadExit = NULL;
	}

	if(m_hNotifyThreadWork)
	{
		CloseHandle(m_hNotifyThreadWork);
		m_hNotifyThreadWork = NULL;
	}
	m_pCallback = NULL;
	m_schSCManager.CloseService();
}

void ThreadFunc(LPVOID p)
{
	CServiceOpt* pThis = static_cast<CServiceOpt*>(p);
	if(pThis)
	{
		pThis->ThreadRun();
	}
}

BOOL CServiceOpt::StartThreadWork(LPVOID lpParam)
{
	if(NULL == m_hThread)
	{
		if(NULL == m_hThreadExit)
			m_hThreadExit = CreateEvent(NULL, FALSE, FALSE, NULL);

		if(NULL == m_hNotifyThreadWork)
			m_hNotifyThreadWork = CreateEvent(NULL, TRUE, FALSE, NULL);

		if(NULL == m_hThreadExit || NULL == m_hNotifyThreadWork)
			return FALSE;
		//创建线程
		m_hThread = (HANDLE)_beginthread(ThreadFunc, 0, this);
		if(NULL == m_hThread)
			return FALSE;
	}

	if(m_hThread && m_hNotifyThreadWork)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(m_hNotifyThreadWork, 0))
		{
			//正在工作，不处理
			return FALSE;
		}

		m_pWorkParam = lpParam;
		SetEvent(m_hNotifyThreadWork);
		return TRUE;
	}

	return FALSE;
}


void CServiceOpt::ReportSetp(DWORD dwStep, DWORD stepVal/* = 0*/)
{
	if(m_pCallback)
	{
		m_pCallback->AnsyRunStep(dwStep, stepVal);
	}
}

void CServiceOpt::ThreadRun()
{
	if(NULL == m_hThreadExit || NULL == m_hNotifyThreadWork)
		return;

	HANDLE hWait[] = {m_hThreadExit, m_hNotifyThreadWork};
	while(1)
	{
		DWORD dwWait = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);
		if(WAIT_OBJECT_0 == dwWait)
			break;
		else if((WAIT_OBJECT_0 + 1) == dwWait)
		{
			DWORD dwCmd = (DWORD)m_pWorkParam;
			dwCmd &= ~RUNCMD_ANSY;

			ReportSetp(RUNSTEP_BEGIN);

			HRESULT hr = S_OK;
			switch(dwCmd)
			{
			case RUNCMD_START:
				hr = StartOrStop(FALSE, TRUE);
				break;
			case RUNCMD_STOP:
				hr = StartOrStop(FALSE, FALSE);
				break;
			case RUNCMD_UNINSTALL:
				hr = UnInstall(FALSE);
				break;
			}

			ReportSetp(RUNSTEP_END, hr);

			ResetEvent(m_hNotifyThreadWork);
		}
	}

	m_hThread = NULL;
}

HRESULT CServiceOpt::QueryServiceStatus(DWORD &dwStatus, LPST_SRVINFO pSrvInfo/* = NULL*/)
{
	if(_tcslen(m_strSrvName) <= 0 || !m_schSCManager)
		return E_INVALIDARG;

	SERVICE_STATUS_PROCESS ssp;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;

	// Get a handle to the service.
	CAutoSHandle schService;
	schService = OpenService( 
		m_schSCManager,         // SCM database 
		m_strSrvName, // name of service 
		SERVICE_QUERY_STATUS|SERVICE_QUERY_CONFIG);  

	if (schService == NULL)
	{
		DWORD dw = GetLastError();
		if(ERROR_SERVICE_DOES_NOT_EXIST == dw)
		{
			return E_NOINTERFACE;
		}

		return E_FAIL;
	}

	if ( !QueryServiceStatusEx( 
		schService, 
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp, 
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded ) )
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError()); 
		return E_FAIL;
	}
	dwStatus = ssp.dwCurrentState;

	if(pSrvInfo)
	{
		pSrvInfo->dwCurrentState = ssp.dwCurrentState;
		pSrvInfo->dwControlsAccepted = ssp.dwControlsAccepted;

		CAutoLocalAlloc<QUERY_SERVICE_CONFIG> lpsc;
		CAutoLocalAlloc<SERVICE_DESCRIPTION> lpsd;
		DWORD dwBytesNeeded, cbBufSize, dwError; 
		
		if( !QueryServiceConfig( 
			schService, 
			NULL, 
			0, 
			&dwBytesNeeded))
		{
			dwError = GetLastError();
			if( ERROR_INSUFFICIENT_BUFFER == dwError )
			{
				cbBufSize = dwBytesNeeded;
				lpsc.Alloc(LMEM_FIXED, cbBufSize);
			}
			else
			{
				printf("QueryServiceConfig failed (%d)", dwError);
				return E_FAIL; 
			}
		}

		if( !QueryServiceConfig( 
			schService, 
			lpsc, 
			cbBufSize, 
			&dwBytesNeeded) ) 
		{
			printf("QueryServiceConfig failed (%d)", GetLastError());
			return E_FAIL;
		}

		if( !QueryServiceConfig2( 
			schService, 
			SERVICE_CONFIG_DESCRIPTION,
			NULL, 
			0, 
			&dwBytesNeeded))
		{
			dwError = GetLastError();
			if( ERROR_INSUFFICIENT_BUFFER == dwError )
			{
				cbBufSize = dwBytesNeeded;
				lpsd.Alloc(LMEM_FIXED, cbBufSize);
			}
			else
			{
				printf("QueryServiceConfig2 failed (%d)", dwError);
				return E_FAIL; 
			}
		}

		if (! QueryServiceConfig2( 
			schService, 
			SERVICE_CONFIG_DESCRIPTION,
			(LPBYTE)(LPSERVICE_DESCRIPTION)lpsd, 
			cbBufSize, 
			&dwBytesNeeded) ) 
		{
			printf("QueryServiceConfig2 failed (%d)", GetLastError());
			return E_FAIL;
		}

		pSrvInfo->dwStartType = lpsc->dwStartType;
		if(lpsc->lpBinaryPathName)
			pSrvInfo->strBinaryPathName = lpsc->lpBinaryPathName;
		if(lpsc->lpDisplayName)
			pSrvInfo->strDisplayName = lpsc->lpDisplayName;
		if(lpsd->lpDescription)
			pSrvInfo->strDescription = lpsd->lpDescription;
	}

	return S_OK;
}

HRESULT CServiceOpt::Start(BOOL bAnsy)
{
	return StartOrStop(bAnsy, TRUE);
}

HRESULT CServiceOpt::Stop(BOOL bAnsy)
{
	return StartOrStop(bAnsy, FALSE);
}

HRESULT CServiceOpt::Install(LPCTSTR lpFile, LPCTSTR lpShowName,DWORD dwDriveType, DWORD dwStartType, LPCTSTR lpDesc)
{
	if(_tcslen(m_strSrvName) <= 0 || !m_schSCManager)
		return E_INVALIDARG;

	RASSERT(lpShowName, E_INVALIDARG);

	CAutoSHandle schService;
	// Create the service.
	schService = CreateService( 
		m_schSCManager,              // SCM database 
		m_strSrvName,              // name of service 
		lpShowName,              // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		dwDriveType, // service type 
		dwStartType,				// start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		lpFile,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 
	if (schService == NULL) 
	{
		printf("CreateService failed (%d)\n", GetLastError()); 
		return E_FAIL;
	}

	if(lpDesc)
	{
		SERVICE_DESCRIPTION sd;
		sd.lpDescription = (LPTSTR)lpDesc;

		if( !ChangeServiceConfig2(
			schService,                 // handle to service
			SERVICE_CONFIG_DESCRIPTION, // change: description
			&sd) )                      // new description
		{
			printf("ChangeServiceConfig2 failed\n");
		}
	}
	return S_OK;
}

HRESULT CServiceOpt::UnInstall(BOOL bAnsy)
{
	if(_tcslen(m_strSrvName) <= 0)
		return E_INVALIDARG;

	if(bAnsy) //异步
	{
		BOOL bOk = StartThreadWork(LPVOID(RUNCMD_ANSY|RUNCMD_UNINSTALL));
		return bOk?S_OK:E_FAIL;
	}
	else //同步
	{
		if(NULL == m_schSCManager)
			return E_FAIL;

		//先停服务
		HRESULT hr = StartOrStop(FALSE, FALSE);
		RFAILED(hr);

		CAutoSHandle  schService;
		schService = ::OpenService(m_schSCManager, m_strSrvName, SERVICE_ALL_ACCESS);	
		if(!schService)
			return E_FAIL;

		if (! DeleteService(schService) ) 
		{
			printf("DeleteService failed (%d)\n", GetLastError());
			return E_FAIL;
		}
		return S_OK;
	}

	return E_FAIL;
}

HRESULT CServiceOpt::WaitServiceState(SC_HANDLE schService, 
									  DWORD dwCheckState, DWORD dwWaitState, DWORD &dwRcState,
									  DWORD dwStartTime, DWORD dwTimeout)
{
	SERVICE_STATUS_PROCESS ssp;
	DWORD dwBytesNeeded;

	if ( !QueryServiceStatusEx( 
		schService, 
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp, 
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded ) )
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError()); 
		return E_FAIL;
	}

	dwRcState = ssp.dwCurrentState;
	if ( ssp.dwCurrentState == dwWaitState )
	{
		return S_OK;
	}

	while ( dwCheckState!=dwWaitState?	//判断比较状态是否和等待状态一直
			ssp.dwCurrentState == dwCheckState:	//不等时：则说明是判断比较状态
			ssp.dwCurrentState != dwCheckState  //相等时：则说明是判断不等于比较状态
				) 
	{
		Sleep( ssp.dwWaitHint );
		if ( !QueryServiceStatusEx( 
			schService, 
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp, 
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded ) )
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError()); 
			return E_FAIL;
		}

		dwRcState = ssp.dwCurrentState;
		if ( ssp.dwCurrentState == dwWaitState )
		{
			return S_OK;
		}

		if ( GetTickCount() - dwStartTime > dwTimeout )
		{
			return CO_E_SERVER_INIT_TIMEOUT;
		}
	}

	return S_OK;
}

HRESULT CServiceOpt::StartOrStop(BOOL bAnsy, BOOL bStart)
{
	if(_tcslen(m_strSrvName) <= 0)
		return E_INVALIDARG;

	if(bAnsy) //异步
	{
		DWORD dwCmd = bStart?(RUNCMD_ANSY|RUNCMD_START):(RUNCMD_ANSY|RUNCMD_STOP);
		BOOL bOk = StartThreadWork(LPVOID(dwCmd));
		return bOk?S_OK:E_FAIL;
	}
	else //同步
	{
		if(NULL == m_schSCManager)
			return E_FAIL;

		CAutoSHandle  schService;
		schService = ::OpenService(m_schSCManager, m_strSrvName, SERVICE_ALL_ACCESS);	
		if(!schService)
			return E_FAIL;

		DWORD dwStartTime = GetTickCount();
		DWORD dwTimeout = m_dwWaitTimeOut; //default 30-second time-out

		HRESULT hr = S_OK;

		DWORD dwRcState = 0;
		// If a start or stop is pending, wait for it.
		hr = WaitServiceState(schService, 
			bStart?SERVICE_START_PENDING:SERVICE_STOP_PENDING,
			bStart?SERVICE_RUNNING:SERVICE_STOPPED,
			dwRcState,
			dwStartTime, dwTimeout);
		if(FAILED(hr))
			return hr;

		// start or stop the service.
		if(bStart)
		{
			if(SERVICE_RUNNING == dwRcState)
				return S_OK;

			if (!StartService(schService, 0, NULL))
			{
				return E_FAIL;
			}
		}
		else
		{
			if(SERVICE_STOPPED == dwRcState)
				return S_OK;

			SERVICE_STATUS ssp;
			if (!ControlService( schService, SERVICE_CONTROL_STOP, &ssp))
			{
				DWORD dwLastError = GetLastError();
				return E_FAIL;
			}
		}

		// Wait for the service to running.
		hr = WaitServiceState(schService, 
			bStart?SERVICE_RUNNING:SERVICE_STOPPED, 
			bStart?SERVICE_RUNNING:SERVICE_STOPPED,
			dwRcState,
			dwStartTime, dwTimeout);
		if(FAILED(hr))
			return hr;

		return S_OK;
	}

	return E_FAIL;
}
