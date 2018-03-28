#include "StdAfx.h"

#include <msapi/mstime.h>
#include <msapi/msapp.h>
#include <msapi/mswinapi.h>

#include <thread/threadex.h>
#include <userguid/GetNetAdapterInfo.h>
#include <network/HttpImplement.h>

#include <JCDeskMsc.h>
#include "DeskTray.h"
#include "resource.h"
#include <shellapi.h>

#define MAIN_TRAY_MSGID		3008
#define MAIN_TRAY_UID		3002

enum TRAY_LEFT_MENU
{
	TLM_EXIT = 1000,
	TLM_REMOVE_ALL_CDROM,
	TLM_SHOW_SEARCH_FS,
	TLM_CHECK_UPDATA,
};

CDeskTray::CDeskTray(void)
{
}


CDeskTray::~CDeskTray(void)
{
}

HRESULT CDeskTray::OnAfterInit(void*)
{
	//连接到托盘服务
	m_pRot->GetObject(ClSID_CTraySrv, re_uuidof(ITraySrv), (void**)&m_pTraySrv.m_p);
	RASSERT(m_pTraySrv, E_FAIL);
	RFAILED(m_pTraySrv->CreateTray(MAIN_TRAY_MSGID, MAIN_TRAY_UID));
	RFAILED(m_pTraySrv->UpdateTray(IDI_MAIN_TRAY_ICON, GetEnvParamString("friendprodname"),TRUE, CMsComBase::GetModuleInstance()));
	m_TrayMenuConnectPoint.Connect(m_pTraySrv.m_p, UTIL::com_ptr<IMSBase>(this));
	m_TrayMsgConnectPoint.Connect(m_pTraySrv.m_p, UTIL::com_ptr<IMSBase>(this));

	return S_OK;
}

HRESULT CDeskTray::OnBeforeUnint()
{
	if (m_pTraySrv)
	{
		m_TrayMenuConnectPoint.DisConnect();
		m_TrayMsgConnectPoint.DisConnect();
		m_pTraySrv->DestroyTray();
		m_pTraySrv = INULL;
	}
	return S_OK;
}

HRESULT CDeskTray::OnAfterStart()
{
	CThreadEx().Do([this]{DoReportActive();});
	return S_OK;
}

HRESULT CDeskTray::OnBeforeStop()
{
	return S_OK;
}


VOID WINAPI CDeskTray::FireConnectBroken()
{
	TerminateProcess( GetCurrentProcess(),0);
}

STDMETHODIMP_(LRESULT) CDeskTray::OnShowLeftButtonMenu(INT x, INT y, BOOL& bHandle) 
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDeskTray::OnShowRightButtonMenu(INT x, INT y, BOOL& bHandle)
{
	//右键菜单
	bHandle = TRUE;
	UTIL::com_ptr<ITrayWnd> pTrayWnd;
	m_pRot->GetObject(ClSID_CTrayWnd, re_uuidof(ITrayWnd), (void**)&pTrayWnd);

	HMENU hPopMenu = CreatePopupMenu();
	DWORD dwPos = GetMessagePos();
	POINT point = {LOWORD(dwPos), HIWORD(dwPos)};

	
// 	AppendMenu(hPopMenu, MF_STRING, TLM_REMOVE_ALL_CDROM, _T("卸载所有虚拟光驱"));
//	AppendMenu(hPopMenu, MF_SEPARATOR, TLM_EXIT, _T(""));

	AppendMenu(hPopMenu, MF_STRING, TLM_SHOW_SEARCH_FS, _T("我要搜文件"));
	AppendMenu(hPopMenu, MF_STRING, TLM_CHECK_UPDATA, _T("检查更新"));
	AppendMenu(hPopMenu, MF_STRING, TLM_EXIT, _T("退出"));

	::TrackPopupMenuEx(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,point.x, point.y, pTrayWnd->GetWndHwnd(), NULL);
	
	DestroyMenu(hPopMenu);
	return 0;
}


STDMETHODIMP_(LRESULT) CDeskTray:: OnMenuCommand(WORD nMenuId, BOOL &bHandle)
{
	
	switch(nMenuId)
	{
	case TLM_EXIT:
		{
			//在退出之前发送退出消息
			Issue( MSCMSG_JCDESK_TRAY_EXIT, (IMsBuffer*)NULL);
			UTIL::com_ptr<IExit> pExit;
			m_pRot->GetObject(CLSID_MsEnv, __uuidof(IExit), (void**)&pExit);
			if (pExit)
				pExit->NotifyExit();

			break;
		}

	case TLM_SHOW_SEARCH_FS:
		{
			CString strJCfs;
			msapi::GetCurrentPath(strJCfs.GetBufferSetLength(MAX_PATH), MAX_PATH);
			strJCfs.ReleaseBuffer();
			strJCfs.Append(_T("\\JCfs.exe"));

			SHELLEXECUTEINFO shellInfo = { 0 };
			shellInfo.cbSize = sizeof( shellInfo );
			shellInfo.lpVerb = _T("open");
			shellInfo.hwnd = NULL;
			shellInfo.lpFile =(LPCWSTR)strJCfs;
			shellInfo.nShow = SW_SHOWNORMAL;
			shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
			ShellExecuteEx( &shellInfo );
		}
		break;

	case TLM_REMOVE_ALL_CDROM:
		//m_pVCDRomCtrl->DisMountAll();
		break;
	case TLM_CHECK_UPDATA:
		{
			CString strUpdata;
			msapi::GetCurrentPath(strUpdata.GetBufferSetLength(MAX_PATH), MAX_PATH);
			strUpdata.ReleaseBuffer();
			strUpdata.Append(_T("\\update.exe"));

			SHELLEXECUTEINFO shellInfo = { 0 };
			shellInfo.cbSize = sizeof( shellInfo );
			shellInfo.lpVerb = _T("open");
			shellInfo.hwnd = NULL;
			shellInfo.lpFile =(LPCWSTR)strUpdata;
			shellInfo.nShow = SW_SHOWNORMAL;
			shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
			ShellExecuteEx( &shellInfo );
			break;
		}
	}
	
	return 0;
}

STDMETHODIMP_(LRESULT) CDeskTray:: OnMenuCommand(UINT nMenuIndex, HMENU hMenu, BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDeskTray:: OnDefaultMenu(BOOL &bHandle)
{
	UTIL::com_ptr<ITrayWnd> pTrayWnd;
	m_pRot->GetObject(ClSID_CTrayWnd, __uuidof(ITrayWnd), (void**)&pTrayWnd);
	if ( pTrayWnd && IsWindow(pTrayWnd->GetWndHwnd()) )
	{
		SendMessage( pTrayWnd->GetWndHwnd(), WM_COMMAND, MAKEWPARAM(TLM_SHOW_SEARCH_FS,0), 0);
	}

	bHandle = TRUE;
	return 0;
}

STDMETHODIMP_(LRESULT) CDeskTray:: OnTrayIconMsg(LPARAM lParam, INT x, INT y, BOOL& bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDeskTray:: OnBalloonClicked(UINT nActionID, BOOL& bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDeskTray:: OnOtherMsg(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP CDeskTray::NotifyExit(bool* bExit /*= NULL*/)
{
	PostQuitMessage(0);
	return S_OK;
}

VOID CDeskTray::DoReportActive()
{
	msdk::network::CUrlParamValueMap ParamValueMap;
	msapi::CApp m_App(CSrvPluginImp::GetEnvParamString("productname"));
	ParamValueMap["apiver"]		=		1;					//当前API版本
	ParamValueMap["hdd"]			=		m_App.GetDiskSerial();
	ParamValueMap["mac"]			=		CGetNetAdapterInfo::GetMac();
	ParamValueMap["osver"]		=		msapi::GetMicroVersion();
	ParamValueMap["wow64"]	=		msapi::IsWindowsX64();
	ParamValueMap["prod"]		=		m_App.GetSetting(_T("prod"),0);
	ParamValueMap["packet"]		=		m_App.GetSetting(_T("packet"),0);
	ParamValueMap["subprod"] =		CSrvPluginImp::GetEnvParamString("subprod");	//当前模块的子产品
	ParamValueMap["idate"]		=		m_App.GetSetting(_T("idate"), msapi::GetCurrentDateStr(FALSE));
	ParamValueMap["itime"]		=		m_App.GetSetting(_T("itime"), msapi::GetCurrentTimeStr(FALSE));
	ParamValueMap["prodver"]	=		m_App.GetVersion();

	msdk::network::CHttpImplement HttpImpl;
	HttpImpl.GetRequest("update.kyan.com.cn", 80, "active" , ParamValueMap);
}