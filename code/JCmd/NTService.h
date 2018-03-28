#ifndef _NTSERVICE_H_
#define _NTSERVICE_H_
#include "ntservmsg.h" 

#define BEGIN_SRV_CTRL_MAP(CLS)\
public:\
	virtual DWORD OnUserControl(DWORD dwControl,DWORD dwEventType,LPVOID lpEventData, LPVOID lpContext)\
	{\
		DWORD dwReult = NO_ERROR;\
		switch(dwControl)\
		{
#define SUBSCRIBE_SRVCTRL(ctrlcode , func)\
			case ctrlcode:\
				{\
					dwReult = func(dwControl , dwEventType , lpEventData , lpContext);\
					break;\
				}
#define END_SRV_CTRL_MAP()\
		}\
		return dwReult;\
	}

/*
BEGIN_SRV_CTRL_MAP(XServeice)
	SUBSCRIBE_SRVCTRL(SERVICE_CONTROL_STOP , OnStop)
	SUBSCRIBE_SRVCTRL(SERVICE_CONTROL_SESSIONCHANGE , OnSessionChanged)
END_SRV_CTRL_MAP()
*/
struct IOnSrvUserControl 
{
	//如果正常请返回 NO_ERROR
	//具体返回值请翻阅 MSDN: HandlerEx 
	virtual HRESULT OnUserControl(DWORD dwControl,DWORD dwEventType,LPVOID lpEventData, LPVOID lpContext)
	{
		return NO_ERROR;
	}

	virtual HRESULT OnRun()  = 0;
	virtual HRESULT OnInit() = 0;
};	

class CNTService
{
public:
    CNTService();
    virtual ~CNTService();

public:
	BOOL Init(const TCHAR* szServiceName);
    BOOL StartService(IOnSrvUserControl* pSrvCtr = NULL);
	SERVICE_STATUS_HANDLE GetServiceStatusHandle();
private:

	VOID SetStatus(DWORD dwState);
	BOOL Initialize();

	VOID LogEvent(WORD wType, 
				  DWORD dwID, 
				  CONST TCHAR* pszS1 = NULL, 
				  CONST TCHAR* pszS2 = NULL, 
				  CONST TCHAR* pszS3 = NULL);


private:
    static VOID  WINAPI  ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static VOID  WINAPI  Handler(DWORD dwOpcode);
	static DWORD WINAPI  HandlerEx(DWORD dwControl,DWORD dwEventType,LPVOID lpEventData, LPVOID lpContext);

private:
    TCHAR				  m_szServiceName[MAX_PATH];
    INT					  m_iMajorVersion;
    INT					  m_iMinorVersion;
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS        m_Status;
    BOOL                  m_bIsRunning;
	BOOL				  m_bWin2kLater;
	BOOL			      m_bNtPlatform;
    HANDLE				  m_hEventSource;
	IOnSrvUserControl*    m_pSrvCtrl;

private:
	static CNTService*    m_pThis;
};

#endif // _NTSERVICE_H_
