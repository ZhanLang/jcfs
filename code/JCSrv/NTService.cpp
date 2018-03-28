#include "StdAfx.h"
#include <windows.h>
#include <stdio.h>
#include "NTService.h"


CNTService* CNTService::m_pThis = NULL;

CNTService::CNTService() : m_pSrvCtrl(NULL)
{
    m_pThis			= this;
	m_bWin2kLater	= FALSE;
	m_bNtPlatform	= FALSE;
	m_iMajorVersion = 1;
	m_iMinorVersion = 0;
	m_hEventSource  = NULL;

	ZeroMemory(m_szServiceName, sizeof(m_szServiceName));
  
    m_hServiceStatus			= NULL;
    m_Status.dwServiceType		= SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState     = SERVICE_STOPPED;

    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP		   | 
								  SERVICE_ACCEPT_SHUTDOWN      | 
								  SERVICE_ACCEPT_SESSIONCHANGE |
								  SERVICE_ACCEPT_NETBINDCHANGE |
								  SERVICE_ACCEPT_PARAMCHANGE   |
								  SERVICE_ACCEPT_PAUSE_CONTINUE|
								  SERVICE_ACCEPT_PRESHUTDOWN   ;

    m_Status.dwWin32ExitCode    = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint       = 0;
    m_Status.dwWaitHint         = 0;
    m_bIsRunning			    = FALSE;
}

BOOL CNTService::Init(const TCHAR* szServiceName)
{
	_tcsncpy_s(m_szServiceName, szServiceName, MAX_PATH);
	return TRUE;
}
CNTService::~CNTService()
{
    if (m_hEventSource) 
	{
        ::DeregisterEventSource(m_hEventSource);
    }
}


void CNTService::LogEvent(WORD wType, DWORD dwID,const TCHAR* pszS1,const TCHAR* pszS2,const TCHAR* pszS3)
{
    const TCHAR* ps[3];
    ps[0] = pszS1;
    ps[1] = pszS2;
    ps[2] = pszS3;

    int iStr = 0;
    for (int i = 0; i < 3; i++) 
	{
        if (ps[i] != NULL) iStr++;
    }
        
    if (!m_hEventSource) 
	{
        m_hEventSource = ::RegisterEventSource(NULL , m_szServiceName);
    }

    if (m_hEventSource) 
	{
        ::ReportEvent(m_hEventSource, wType, 0, dwID, NULL,   iStr, 0, ps, NULL);
    }
}

BOOL CNTService::StartService(IOnSrvUserControl* pSrvCtrl /*=NULL*/)
{
	m_pSrvCtrl = pSrvCtrl;

    SERVICE_TABLE_ENTRY st[] = 
	{
        {m_szServiceName, ServiceMain},
        {NULL, NULL}
    };
	
    return ::StartServiceCtrlDispatcher(st);
}


VOID CNTService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	CNTService* pService = m_pThis;

	OSVERSIONINFOEX osvi  = {0};
	BOOL bOsVersionInfoEx = FALSE;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
		{
			return ;
		}
	}

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId )
	{
		pService->m_bNtPlatform = TRUE;

		if ((osvi.dwMajorVersion == 5 && osvi.dwMinorVersion > 0) || (osvi.dwMajorVersion > 5))
		{
			m_pThis->m_bWin2kLater = TRUE;
		}
	}


	if(pService->m_bWin2kLater)
	{
		pService->m_Status.dwControlsAccepted |= SERVICE_ACCEPT_SESSIONCHANGE;
		pService->m_Status.dwControlsAccepted |= SERVICE_ACCEPT_POWEREVENT;
		pService->m_Status.dwControlsAccepted |= SERVICE_ACCEPT_HARDWAREPROFILECHANGE;
	}
	
    pService->m_Status.dwCurrentState = SERVICE_START_PENDING;

	if (pService->m_bWin2kLater)
	{
		pService->m_hServiceStatus = RegisterServiceCtrlHandlerEx(pService->m_szServiceName , HandlerEx , pService);
	}
    else
	{
		pService->m_hServiceStatus = RegisterServiceCtrlHandler(pService->m_szServiceName , Handler);
	}
	
		
	if (pService->m_hServiceStatus  == 0)
	{
		return ;
	}

    if (pService->m_hServiceStatus == NULL) 
	{
        pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
        return;
    }

  
    if (pService->Initialize()) 
	{
        pService->m_bIsRunning				 = TRUE;
        pService->m_Status.dwWin32ExitCode	 = 0;
        pService->m_Status.dwCheckPoint		 = 0;
        pService->m_Status.dwWaitHint		 = 0;
		if (pService->m_pSrvCtrl)
		{
			pService->m_pSrvCtrl->OnSrvRun();
		}
    }

    pService->SetStatus(SERVICE_STOPPED);
}


VOID CNTService::SetStatus(DWORD dwState)
{
    m_Status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_Status);
}


BOOL CNTService::Initialize()
{
    SetStatus(SERVICE_START_PENDING);
    BOOL bResult = FALSE;

	if (m_pThis->m_pSrvCtrl)
	{
		bResult = m_pThis->m_pSrvCtrl->OnSrvInit(); 
	}
     
    
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;

    if (bResult != S_OK) 
	{
        LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINIT);
        SetStatus(SERVICE_STOPPED);
        return FALSE;    
    }
    
    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STARTED);
    SetStatus(SERVICE_RUNNING);

    return TRUE;
}

VOID CNTService::Handler(DWORD dwOpcode)
{
   HandlerEx(dwOpcode , 0 , NULL , NULL);
}
 
DWORD WINAPI CNTService::HandlerEx( DWORD dwOpcode,DWORD dwEventType,LPVOID lpEventData, LPVOID lpContext)
{
	CNTService* pService = m_pThis;

	if (pService == NULL)
	{
		return 0;
	}
	
	if (pService->m_pSrvCtrl == NULL)
	{
		return 1;
	}
	

	pService->m_pSrvCtrl->OnUserControl(dwOpcode , dwEventType , lpEventData , lpContext);
	//::SetServiceStatus(pService->m_hServiceStatus, &pService->m_Status);
	return NO_ERROR;
}

SERVICE_STATUS_HANDLE CNTService::GetServiceStatusHandle()
{
	return m_hServiceStatus;
}