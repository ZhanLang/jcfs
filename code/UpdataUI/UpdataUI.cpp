// UpdataUI.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "UpdataUI.h"
#include "UIAnimatRotate.h"
#include <process/ParseCommand.h>
#include <network/HttpImplement.h>
#include <json/json.h>
#include "dlgExit.h"
#include <shellapi.h>
#include <msapi/mswinapi.h>
#include <msapi/msapp.h>
#include <util/vercmp.h>
#define WM_EXECUTE_MSG (WM_USER + 55648)


enum WORK_STATE
{
	WORK_STATE_CHECKING = 1,
};

enum PAGE_INDEX
{
	PAGE_INDEX_CHECKING,		//正在检测
	PAGE_INDEX_NET_ERROR,		//网络异常
	PAGE_INDEX_IS_NEW,			//已经是最新版本
	PAGE_INDEX_CHECK_NEW,		//检测到新版本
	PAGE_INDEX_UPDATING,		//正在升级
	PAGE_INDEX_ERROR,			//升级失败

};

// DUI_BEGIN_MESSAGE_MAP(CUpdataUI,WindowImplBase)
// 	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
// 	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT,OnWinInit)
// 	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelChange)
// 	DUI_ON_MSGTYPE(DUI_MSGTYPE_TABSELECT,OnTabSel)
// 	//DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMDBCLICK, OnItemDBClick)
// DUI_END_MESSAGE_MAP()

#define MAIN_TRAY_MSGID		2008

#define MAIN_TRAY_UID		2002

#define UPDATA_TIMER		10547
CUpdataUI::CUpdataUI():m_workCenter(this)
{
	m_bDownLoad = FALSE;
	m_dlProg = 0;
	m_nTaryIcon = IDI_ICON1;
}



STDMETHODIMP CUpdataUI::Init(void*)
{
	CPaintManagerUI::SetInstance(GetPorcessInstance());
	CPaintManagerUI::SetResourceDll(GetModuleInstance());


#ifdef DEBUG
	TCHAR szTemp[MAX_PATH] = { 0 };
	msapi::GetCurrentPath(szTemp, MAX_PATH);
	CDuiString path = szTemp;
	path.Append(_T("\\updata_skin\\"));

	CPaintManagerUI::SetResourceType(UILIB_FILE);
	CPaintManagerUI::SetResourcePath(path);
#else
	CString strZipUI;
	GetTempPath(MAX_PATH, strZipUI.GetBufferSetLength(MAX_PATH));
	strZipUI.ReleaseBuffer();
	strZipUI.Append(_T("\\JCDeskUn.zip"));

	msapi::ExportResourceToFile(g_hinstance, IDR_SKIN_MAIN, _T("ZIPRES"), strZipUI);

	CPaintManagerUI::SetResourceZip(strZipUI);
	CPaintManagerUI::SetResourceType(UILIB_ZIP);
#endif

		
	return S_OK;
}

STDMETHODIMP CUpdataUI::Uninit()
{
	CPaintManagerUI::Term();

	if (m_pTraySrv)
	{
		m_TrayMenuConnectPoint.DisConnect();
		m_TrayMsgConnectPoint.DisConnect();
		m_pTraySrv->DestroyTray();
		m_pTraySrv = INULL;
	}


	return S_OK;
}

STDMETHODIMP CUpdataUI::Start()
{
	HRESULT hResult = S_OK;

	CParseCommand parseCmd(TRUE);

	CString sName = GetEnvParamString("friendprodname");
	if( !Create((HWND)parseCmd.GetArgInt(_T("hwnd"), NULL), sName, UI_WNDSTYLE_FRAME^WS_MAXIMIZEBOX, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 600, 800))
		return E_FAIL;

	
	
	//连接到托盘服务
	m_pRot->GetObject(ClSID_CTraySrv, re_uuidof(ITraySrv), (void**)&m_pTraySrv.m_p);
	RASSERT(m_pTraySrv, E_FAIL);
	RFAILED(m_pTraySrv->CreateTray(MAIN_TRAY_MSGID, MAIN_TRAY_UID));
	m_TrayMenuConnectPoint.Connect(m_pTraySrv.m_p, UTIL::com_ptr<IMSBase>(this));
	m_TrayMsgConnectPoint.Connect(m_pTraySrv.m_p, UTIL::com_ptr<IMSBase>(this));

	m_pTraySrv->ShowTray(false);
	SetTimer(m_hWnd, UPDATA_TIMER, 100,NULL);
	
	return S_OK;
}

STDMETHODIMP CUpdataUI::Stop()
{
	return S_OK;
}

CDuiString CUpdataUI::GetSkinFolder()
{
	return _T("");
}

CDuiString CUpdataUI::GetSkinFile()
{
	return _T("main.xml");
}

 CDuiString CUpdataUI::GetZIPFileName() const 
{
	return m_strSkin;
}

LPCTSTR CUpdataUI::GetWindowClassName(void) const
{
	return _T("UI_JICER_UPDATA");
}


LPCTSTR  CUpdataUI::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_SKIN_MAIN);
}

CControlUI* CUpdataUI::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("AnimationRotate")) == 0)
	{
		return new CAnimatRotate;
	}
	return NULL;


}
void CUpdataUI::OnTabSel(TNotifyUI& msg)
{
}

void CUpdataUI::OnWorkStatus(LPARAM lParam)
{
	if ( !m_pDlMgr )
	{
		CreateInstance(CLSID_DownloadMgr, NULL, re_uuidof(IDownloadMgr), (void**)&m_pDlMgr.m_p);
		if ( !(m_pDlMgr && SUCCEEDED(m_pDlMgr->OpenDownloadMgr(this, GetEnvParamString("dlmgr")))))
		{
			Post([this]{this->ShowPage(PAGE_INDEX_NET_ERROR);});
			return ;
		}
	}



	//检查更新
	if ( lParam == WORK_STATE_CHECKING )
	{
		Sleep(500);
		network::CHttpImplement http;
		network::CUrlParamValueMap urlParamMap;
		TCHAR szDiskSer[MAX_PATH] = { 0 };
		TCHAR szVer[ MAX_PATH ] = { 0 };
		TCHAR szOper[MAX_PATH] = { 0 };

		msapi::CApp softApp(GetEnvParamString("productname"));

		urlParamMap["prouctid"] = softApp.GetSetting(_T("prouctid"), 0);
		urlParamMap["packetid"] = softApp.GetSetting(_T("packetid"), 0);
		urlParamMap["version"]	= softApp.GetVersion(szVer, MAX_PATH);
		urlParamMap["oper"]		= softApp.GetSetting(_T("oper"), _T("0"), szOper, MAX_PATH);
		urlParamMap["hdd"]	    = msapi::CApp::GetDiskSerial(szDiskSer, MAX_PATH);

		if ( http.GetRequest("api.kyan.com.cn", 80, "jcdisk/update", urlParamMap) )
		{
			Post([&]{this->ShowPage(PAGE_INDEX_NET_ERROR);});
			return ;
		}

		Json::Value root;
		if ( !Json::Reader().parse((const char*)http.GetRequestData(), root) )
		{
			Post([&]{this->ShowPage(PAGE_INDEX_NET_ERROR);});
			return ;
		}

		Json::Value retCode =  root["code"];
		if (retCode.type() != Json::intValue )
		{
			Post([&]{this->ShowPage(PAGE_INDEX_NET_ERROR);});
			return ;
		}

		//已是最新版本
		if ( retCode.asInt() == 0 )
		{
			Post([this]{this->ShowPage(PAGE_INDEX_IS_NEW);});
			return ;
		}

		//检测到最新版本
		if (  retCode.asInt() == 1 )
		{
			Json::Value Msg = root["msg"];
			m_strName = Msg["name"].asWString().c_str();
			m_strUrl = Msg["url"].asWString().c_str();
			m_strVer = Msg["version"].asWString().c_str();

			TCHAR szVersion[ MAX_PATH ] = { 0 };
			msapi::CApp(GetEnvParamString("productname")).GetVersion(szVersion,MAX_PATH);
			if ( CVerCmp::CompareStr(m_strVer, szVersion ) > 0)
			{
				GetTempPath(MAX_PATH, m_strSave.GetBufferSetLength(MAX_PATH));
				m_strSave.ReleaseBuffer();
				m_strSave.Append(m_strName);

				Post([this]{this->ShowPage(PAGE_INDEX_CHECK_NEW);});
			}
			else
			{
				Post([this]{this->ShowPage(PAGE_INDEX_IS_NEW);});
			}

			
			return ;
		}
	}

	return ;
}



VOID CUpdataUI::ShowPage(INT nPage)
{
	CTabLayoutUI* pTabUI = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tab_msg")));
	if ( pTabUI )
	{
		pTabUI->SelectItem(nPage);
		RECT R = m_pm.GetRoot()->GetPos();
		m_pm.Invalidate(R);
	}
}

void CUpdataUI::OnWinInit(TNotifyUI& msg)
{

}

void CUpdataUI::OnSelChange(TNotifyUI& msg)
{

}

void CUpdataUI::OnClick(TNotifyUI& msg)
{
	if ( msg.pSender->GetName() == _T("btn_retry"))
	{
		m_workCenter.Dispatch((LPARAM)WORK_STATE_CHECKING);
	}

	if ( msg.pSender->GetName() == _T("closebtn"))
	{
		if ( m_bDownLoad )
		{
			CDlgExit dlg;
			dlg.Create(m_hWnd, GetEnvParamString("friendprodname"), UI_WNDSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_TOOLWINDOW, 0, 0, 500, 262);
			dlg.CenterWindow();
			UINT uRet = dlg.ShowModal();

			if ( uRet == 1 )
			{
				if ( dlg.NeedExit() )
				{
					this->Close();
					PostQuitMessage(0);
				}
				else{
					this->ShowWindow(false,false);
					m_pTraySrv->ShowTray(true);
				}
				
				return ;
			}
			return;
		}
		else{
			this->Close();
			PostQuitMessage(0);
		}
	
		
	}

	if ( msg.pSender->GetName() == _T("btn_ok"))
	{
		if ( m_pDlMgr )
		{
			DWORD dwTask = m_pDlMgr->CreateDownloadTask(m_strUrl, m_strSave, FALSE, FALSE);
			if ( !(dwTask != -1 && SUCCEEDED(m_pDlMgr->StartTask(dwTask))) )
				Post([&]{ShowPage(PAGE_INDEX_NET_ERROR);});
			else
				Post([&]{ShowPage(PAGE_INDEX_UPDATING);});
		}
	}

	if ( msg.pSender->GetName() == _T("btn_no"))
	{
		this->Close();
		PostQuitMessage(0);
	}
}


LRESULT CUpdataUI::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lResult = WindowImplBase::OnCreate(uMsg, wParam, lParam, bHandled);
	SetIcon(108);
	CenterWindow();

	m_workCenter.Dispatch((LPARAM)WORK_STATE_CHECKING);

	return lResult;
}

LRESULT CUpdataUI::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lResult = WindowImplBase::OnClose(uMsg, wParam, lParam, bHandled);
	this->ShowWindow(false,false);

	return lResult;
}


LRESULT CUpdataUI::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	switch(uMsg)
	{
	case WM_EXECUTE_MSG:
		auto func = (std::function<void()> *)lParam;
		(*func)();
		delete func;
		return 0;
		break;
	}

	switch( uMsg)
	{
	case WM_TIMER:
		{
			if ( wParam == UPDATA_TIMER)
			{
				if ( m_pTraySrv )
				{
					CString sMsg;
					sMsg.Format(_T("%s : %d%%"), GetEnvParamString("friendprodname"), m_dlProg);
					if ( m_nTaryIcon > IDI_ICON12)
						m_nTaryIcon = IDI_ICON1;

					RFAILED(m_pTraySrv->UpdateTray(m_nTaryIcon,sMsg,TRUE, CMsComBase::GetModuleInstance()));

					m_nTaryIcon ++;
				}
			}

			break;
		}
	}
	

	
	return 0;
}

LRESULT CUpdataUI::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_EXECUTE_MSG, WM_EXECUTE_MSG, PM_REMOVE)) {
		DispatchMessage(&msg);
	}



	if ( m_pDlMgr )
	{
		m_pDlMgr->StopTask( -1 );
		m_pDlMgr->RemoveTask( -1 );
		m_pDlMgr->CloseDownloadMgr();
		m_pDlMgr = INULL;
	}

	UTIL::com_ptr<IExit> pExit;
	m_pRot->GetObject(CLSID_MsEnv, __uuidof(IExit), (void**)&pExit.m_p);
	if ( pExit )
	{
		pExit->NotifyExit();
	}

	return 0;
}


void CUpdataUI::Post(const std::function<void()> &f) {
	auto func = new std::function<void()>(f);
	PostMessage( WM_EXECUTE_MSG, 0, (LPARAM)func);
}

STDMETHODIMP CUpdataUI::OnDownloadNotify(DL_NotifyEvnet notifyType, DWORD dwTaskId)
{
	if ( DL_NotifyEvent_Error == notifyType)
	{
		m_bDownLoad = FALSE;
		Post([this]{ShowPage(PAGE_INDEX_NET_ERROR);});
		return 0;
	}

	if ( DL_NotifyEvnet_DownloadStart == notifyType)
	{
		m_bDownLoad = TRUE;
		Post([this]{ShowPage(PAGE_INDEX_UPDATING);});
		return 0;
	}

	if ( DL_NotifyEvnet_Downloading == notifyType)
	{
		UTIL::com_ptr<IDownloadTask> pTask = m_pDlMgr->GetDownloadTask(dwTaskId);
		if ( pTask )
		{
			FLOAT flPercent = FLOAT(pTask->GetCurrentFileSize()) / FLOAT(pTask->GetTotalFileSize());
			flPercent = flPercent * FLOAT(100);;
			m_dlProg = flPercent;
			Post([=]{SetDlProgress((int)flPercent);});
		}
		
		return 0;
	}
	if ( DL_NotifyEvnet_Complete == notifyType)
	{
		m_bDownLoad = FALSE;
		m_pTraySrv->ShowTray(FALSE);
		Post([this]{DlFinish();});
	}
	return S_OK;
}

VOID CUpdataUI::SetDlProgress(INT prog)
{
	if ( !prog )
		return;
	
	CProgressUI* PrgrsUI = static_cast<CProgressUI*>(m_pm.FindControl(_T("prgrs")));
	PrgrsUI->SetValue(prog);

	CLabelUI* pLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("inst_lbl_per")));
	if ( pLabel )
	{
		
		CDuiString strTmp;
		strTmp.Format(_T("%d"), (int)prog);
		pLabel->SetText(strTmp);

		CDuiRect rt;
		rt.top = 152;
		rt.bottom = 171;
		rt.left = 10;

		rt.left += ( 450 * prog) / 100;
		rt.right = rt.left + 24;
		pLabel->SetPos(rt);
		pLabel->SetVisible(true);
	}
}



STDMETHODIMP_(LRESULT) CUpdataUI::OnShowLeftButtonMenu(INT x, INT y, BOOL& bHandle) 
{
	this->ShowWindow(true, true);
	if ( m_pTraySrv )
		m_pTraySrv->ShowTray(false);
	
	return 0;
}

STDMETHODIMP_(LRESULT) CUpdataUI::OnShowRightButtonMenu(INT x, INT y, BOOL& bHandle)
{
	//右键菜单
	this->ShowWindow(true, true);
	if ( m_pTraySrv )
		m_pTraySrv->ShowTray(false);
	return 0;
}



STDMETHODIMP_(LRESULT) CUpdataUI:: OnMenuCommand(WORD nMenuId, BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CUpdataUI:: OnMenuCommand(UINT nMenuIndex, HMENU hMenu, BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CUpdataUI:: OnDefaultMenu(BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CUpdataUI:: OnTrayIconMsg(LPARAM lParam, INT x, INT y, BOOL& bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CUpdataUI:: OnBalloonClicked(UINT nActionID, BOOL& bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CUpdataUI:: OnOtherMsg(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandle)
{
	return 0;
}

VOID CUpdataUI::DlFinish()
{
	SHELLEXECUTEINFO ShellInfo = { 0 };
	ShellInfo.cbSize = sizeof( ShellInfo );
	ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellInfo.hwnd = NULL;
	ShellInfo.lpVerb = NULL;

	ShellInfo.lpFile = m_strSave;
	ShellInfo.lpParameters = L"";
	ShellInfo.lpDirectory = NULL;
	ShellInfo.nShow = SW_SHOW;
	ShellInfo.hInstApp = NULL;

	if (ShellExecuteEx(&ShellInfo) )
		Close(1);
	else
		Post([this]{ShowPage(PAGE_INDEX_ERROR);});

}
