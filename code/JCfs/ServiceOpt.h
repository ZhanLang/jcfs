#pragma once
#include <string>
using namespace std;
class CAutoSHandle
{
public:
	CAutoSHandle()
		:m_schService(NULL)
	{
	}

	virtual ~CAutoSHandle()
	{
		CloseService();
	}

	void operator =(SC_HANDLE schService)
	{
		CloseService();
		m_schService = schService;
	}

	BOOL CloseService()
	{
		if(m_schService)
		{
			CloseServiceHandle(m_schService);
			m_schService = NULL;
		}
		return TRUE;
	}

	operator SC_HANDLE() const {return m_schService;}

	SC_HANDLE m_schService;
};

template <class T>
class CAutoLocalAlloc
{
public:
	CAutoLocalAlloc():m_p(NULL){}
	virtual ~CAutoLocalAlloc()
	{
		Free();
	}

	HLOCAL Alloc(__in UINT uFlags, __in SIZE_T uBytes)
	{
		Free();
		m_p = (T*)LocalAlloc(uFlags, uBytes);
		return m_p;
	}

	void Free()
	{
		if(m_p)
		{
			LocalFree(m_p);
			m_p = NULL;
		}
	}

	operator T*() const {return m_p;}
	T* operator -> () const {return m_p;}

	T* m_p;
};

enum
{

};

enum
{
	RUNSTEP_BEGIN = 0,
	RUNSTEP_STEP,
	RUNSTEP_END,
};

struct ST_SRVINFO
{
	DWORD   dwStartType;
	DWORD   dwCurrentState;
	DWORD   dwControlsAccepted;
	std::basic_string<TCHAR> strBinaryPathName;
	std::basic_string<TCHAR> strDisplayName;
	std::basic_string<TCHAR> strDescription;
};
typedef ST_SRVINFO* LPST_SRVINFO;

interface IServiceOptCallback
{
	virtual void AnsyRunStep(DWORD dwStep, DWORD stepVal = 0) = 0;
};

#define DEFAULT_WAIT_TIMEOUT	30000

#define RUNCMD_ANSY			0x1000
#define RUNCMD_START		0x0001
#define RUNCMD_STOP			0x0002
#define RUNCMD_UNINSTALL	0x0003

class CServiceOpt
{
public:
	CServiceOpt(void);
	CServiceOpt(LPCTSTR lpName);
	virtual ~CServiceOpt(void);

	void	SetServiceName(LPCTSTR lpName);
	LPCTSTR GetServiceName();
	HRESULT Init(IServiceOptCallback* pCallback = NULL, DWORD dwWaitTimeOut = DEFAULT_WAIT_TIMEOUT);
	void	Uninit();

	HRESULT QueryServiceStatus(DWORD &dwStatus, LPST_SRVINFO pSrvInfo = NULL);
	HRESULT Start(BOOL bAnsy);
	HRESULT Stop(BOOL bAnsy);
	HRESULT Install(LPCTSTR lpFile, LPCTSTR lpShowName, DWORD dwDriveType, DWORD dwStartType, LPCTSTR lpDesc);
	HRESULT UnInstall(BOOL bAnsy);

	void	ThreadRun();

private:
	CAutoSHandle m_schSCManager;
	TCHAR	m_strSrvName[MAX_PATH];
	DWORD	m_dwWaitTimeOut;
	IServiceOptCallback*	m_pCallback;

	HANDLE  m_hThread;
	HANDLE  m_hThreadExit;
	HANDLE  m_hNotifyThreadWork;

	LPVOID  m_pWorkParam;

	BOOL	StartThreadWork(LPVOID pWorkParam);
	void	ReportSetp(DWORD dwStep, DWORD stepVal = 0);

	HRESULT StartOrStop(BOOL bAnsy, BOOL bStart);
	HRESULT WaitServiceState(SC_HANDLE schService, 
		DWORD dwCheckState, DWORD dwWaitState, DWORD &dwRcState,
		DWORD dwStartTime, DWORD dwTimeout);
};
