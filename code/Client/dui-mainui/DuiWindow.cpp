#include "stdafx.h"
#include "Wtsapi32.h"

#include <process/ParseCommand.h>

#include "DuiWindow.h"
#include "resource.h"
#include "WTLVirtualList.h"
#include "GifControlUI.h"


#include "shellcontextmenu/ShellContextMenu.h"
#include "KYanBuildControl.h"
#include "ServiceOpt.h"
#define MAIN_TRAY_MSGID		2008
#define MAIN_TRAY_UID		2002



static menu_command PathMenuDef[] = 
{
	{_T("我的电脑"), _T("")},
	{_T("我的桌面"), _T("CSIDL_DESKTOPDIRECTORY")},
	{_T(""), _T("")},
};

CDuiWindow::CDuiWindow(void)
{
	//m_pResultList = NULL;
	m_lpResourceZIPBuffer = NULL;
	
	
	
	
}


CDuiWindow::~CDuiWindow(void)
{
}

HRESULT CDuiWindow::OnAfterInit(void*)
{	
	//创建托盘
	m_pRot->GetObject(ClSID_CTraySrv, re_uuidof(ITraySrv), (void**)&m_pTraySrv.m_p);
	RASSERT(m_pTraySrv, E_FAIL);
	RFAILED(m_pTraySrv->CreateTray(MAIN_TRAY_MSGID, MAIN_TRAY_UID));


	RFAILED(m_pTraySrv->UpdateTray(IDI_MAIN_TRAY_ICON,GetEnvParamString("friendprodname"),TRUE, CMsComBase::GetModuleInstance()));
	m_TrayMenuConnectPoint.Connect(m_pTraySrv.m_p, UTIL::com_ptr<IMSBase>(this));
	m_TrayMsgConnectPoint.Connect(m_pTraySrv.m_p, UTIL::com_ptr<IMSBase>(this));

	//WTSRegisterSessionNotification(this->GetHWND(), NOTIFY_FOR_THIS_SESSION);



	m_pRot->GetObject(CLSID_SignalApp, re_uuidof(IMSBase), (void**)&m_pSignalApp.m_p);
	RASSERT(m_pSignalApp, E_FAIL);
	m_SingleAppConnectPoint.Connect(m_pSignalApp, UTIL::com_ptr<IMSBase>(this));

	//////////////////////////////////////////////////////////////////////////
	//
	CSkinManager::Init();	// 初始化皮肤管理器
	tstring strSkinPath = ZYM::CPath::GetAppPath() + _T("Skins\\");	// 设置皮肤文件夹路径
	CSkinManager::GetInstance()->SetSkinPath(strSkinPath.c_str());
	CSkinManager::GetInstance()->LoadConfigXml();	// 加载皮肤列表配置文件

	//////////////////////////////////////////////////////////////////////////
	//
	CPaintManagerUI::SetInstance(CMsComBase::GetModuleInstance());
	CPaintManagerUI::SetResourceDll(this->GetModuleInstance());
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));

#ifdef DEBUG
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
#else
	HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), MAKEINTRESOURCE(IDR_SKIN_DB), _T("DTATA"));
	if( hResource == NULL )
		return 0L;
	DWORD dwSize = 0;
	HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
	if( hGlobal == NULL ) 
	{
		::FreeResource(hResource);
		return E_FAIL;
	}

	dwSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
	if( dwSize == 0 )
		return 0L;
	m_lpResourceZIPBuffer = new BYTE[ dwSize ];
	if (m_lpResourceZIPBuffer != NULL)
	{
		::CopyMemory(m_lpResourceZIPBuffer, (LPBYTE)::LockResource(hGlobal), dwSize);
	}
	
	CPaintManagerUI::SetResourceZip(m_lpResourceZIPBuffer, dwSize);
	::FreeResource(hResource);
#endif

	return S_OK;
}

HRESULT CDuiWindow::OnBeforeUnint()
{
	//WTSUnRegisterSessionNotification(this->GetHWND());
	CSkinManager::UnInit();	// 反初始化皮肤管理器
	if (m_pTraySrv)
	{
		m_TrayMenuConnectPoint.DisConnect();
		m_TrayMsgConnectPoint.DisConnect();
		m_pTraySrv->DestroyTray();
		m_pTraySrv = INULL;
	}

	if (m_pSignalApp)
	{
		m_SingleAppConnectPoint.DisConnect();
		m_pSignalApp = INULL;
	}

	if (m_lpResourceZIPBuffer)
	{
		delete[] m_lpResourceZIPBuffer;
		m_lpResourceZIPBuffer = NULL;
	}
	return S_OK;
}

HRESULT CDuiWindow::OnAfterStart()
{
	HWND hWnd= Create(NULL, GetEnvParamString("friendprodname"), UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW );
	this->SetIcon(IDI_ICON_TOOLBAR);
	CenterWindow();

	CParseCommand ParseCmd(TRUE);
	LPCTSTR lpszFilterPath = ParseCmd.GetArg(_T("filter"));

	CreateTableItem(lpszFilterPath);
	
	LPCWSTR lpszComx3NameSpace = GetEnvParamString("comx3");
	USES_CONVERSION;
	//CreateComx3(m_pRot, COMX3_SHELL_CONTEXTMENU_DEFID_UI, COMX3_SHELL_CONTEXTMENU_MASKID_UI,W2A(lpszComx3NameSpace));

	//需要调出界面
	ShowActiveWindow();

	return S_OK;
}

HRESULT CDuiWindow::OnBeforeStop()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = NULL;

	ReleaseComx3();
	return S_OK;
}

STDMETHODIMP_(LRESULT) CDuiWindow::OnShowLeftButtonMenu(INT x, INT y, BOOL& bHandle) 
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow::OnShowRightButtonMenu(INT x, INT y, BOOL& bHandle)
{
	CSkinMenu skinPopMenu = m_RClickPopMenu.GetSubMenu(2);
// 	skinPopMenu.SetBgPic(_T("Menu\\menu_left_bg.png"), _T("Menu\\menu_right_bg.png"));
// 	skinPopMenu.SetSelectedPic(_T("Menu\\menu_selected.png"));
// 	skinPopMenu.SetSepartorPic(_T("Menu\\menu_separtor.png"));
// 	skinPopMenu.SetArrowPic(_T("Menu\\menu_arrow.png"));
// 	skinPopMenu.SetCheckPic(_T("Menu\\menu_check.png"));
// 	skinPopMenu.SetTextColor(RGB(0, 20, 35));
// 	skinPopMenu.SetSelTextColor(RGB(254, 254, 254));

	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));
	//::TrackPopupMenu()

	//skinPopMenu.CheckMenuItem(ID_TrayMenu_SearchFlord, IsInstallKYanSellEx() ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
	//skinPopMenu.CheckMenuItem(ID_TrayMenu_MonitorFileChange, IsInstallLdrSrv() ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
	
	skinPopMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow:: OnMenuCommand(WORD nMenuId, BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow:: OnMenuCommand(UINT nMenuIndex, HMENU hMenu, BOOL &bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow:: OnDefaultMenu(BOOL &bHandle)
{
	bHandle = TRUE;
	ShowActiveWindow();
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow:: OnTrayIconMsg(LPARAM lParam, INT x, INT y, BOOL& bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow:: OnBalloonClicked(UINT nActionID, BOOL& bHandle)
{
	return 0;
}

STDMETHODIMP_(LRESULT) CDuiWindow:: OnOtherMsg(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandle)
{
	if (msg == ID_VISIT_WEBSITE)
	{
		OutputDebugString(_T("AAA"));
	}
	return 0;
}
STDMETHODIMP CDuiWindow::OnSingleAppMsg(LPCWSTR lpArgXml , BOOL& bHandle)
{
	bHandle = TRUE;

	//需要调出界面
	ShowActiveWindow();
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////

LPCTSTR CDuiWindow::GetWindowClassName() const
{ 
	return _T("ScanMainForm"); 
};
// 
UINT CDuiWindow::GetClassStyle() const
{ 
	return CS_DBLCLKS; 
};
// 
void CDuiWindow::OnFinalMessage(HWND /*hWnd*/) 
{ 
	//delete this;
};

// void CDuiWindow::Notify(TNotifyUI& msg)
// {
// 	//版本访问主页
// 	if (msg.pSender->GetName() == _T("version_label") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		SendMessage(WM_COMMAND, ID_MENU_VISIT_WEBSITE);
// 	}
// 
// 	if(msg.pSender == m_pResultList)
// 	{
// 		if(msg.sType == DUI_MSGTYPE_ITEMDBCLICK)
// 		{
// 			CString strPath;
// 			RASSERT(GetCurrentSelPath(strPath), );
// 			RASSERT(PathFileExists(strPath),);
// 			PostMessage(WM_COMMAND, ID_1000, 0);
// 		}
// 		if(msg.sType == DUI_MSGTYPE_RBUTTONUP)
// 		{
// 			CString strSelPath;
// 			RASSERT(GetCurrentSelPath(strSelPath),);
// 			RASSERT(PathFileExists(strSelPath),);
// 			BOOL isDirect = PathIsDirectory(strSelPath);
// 
// 			CSkinMenu skinPopMenu = m_RClickPopMenu.GetSubMenu(!isDirect? 0:1);
// 
// 			InitMenu(skinPopMenu);
// 			DWORD dwPos = GetMessagePos();
// 			CPoint point(LOWORD(dwPos), HIWORD(dwPos));
// 			skinPopMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);
// 		}
// 	}
// 	if (msg.pSender->GetName() == _T("search_input_edit"))
// 	{
// 		if (msg.sType == DUI_MSGTYPE_TEXTCHANGED)
// 		{
// 			DoSearch(TRUE);
// 		}
// 	}
// 	if (msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		OnClick(msg);
// 	}
// 	if (msg.sType == DUI_MSGTYPE_SELECTCHANGED)
// 	{
// 		if (msg.pSender->GetName() == _T("option_match_word") ||
// 			msg.pSender->GetName() == _T("option_match_case") ||
// 			msg.pSender->GetName() == _T("option_include_sub_dir") ||
// 			msg.pSender->GetName() == _T("option_match_Exclude_System") ||
// 			msg.pSender->GetName() == _T("option_match_Exclude_Hidden")
// 			) 
// 		{
// 			DoSearch(TRUE);
// 		}
// 
// 		//扩展名过滤条件
// 		if (msg.pSender->GetName() == _T("filter_all") || 
// 			msg.pSender->GetName() == _T("filter_dir") || 
// 			msg.pSender->GetName() == _T("filter_pic") || 
// 			msg.pSender->GetName() == _T("filter_doc") || 
// 			msg.pSender->GetName() == _T("filter_video") || 
// 			msg.pSender->GetName() == _T("filter_audio") || 
// 			msg.pSender->GetName() == _T("filter_zip") || 
// 			msg.pSender->GetName() == _T("filter_exe"))
// 		{
// 			m_strextension = msg.pSender->GetUserData();
// 			DoSearch(TRUE);
// 		}
// 	}
// 	if (msg.pSender == m_pPathEdit)
// 	{
// 		if (msg.sType == _T("current_text_change"))
// 		{
// 			DoSearch(TRUE);
// 		}
// 	}
// 
// 	if (msg.pSender->GetName() == _T("button_set_current_path") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		//弹出菜单
// 		m_pathMenuCmd.clear();
// 		CSkinMenu popMenu;
// 		popMenu.CreatePopupMenu();
// 		InitMenu(popMenu);
// 		
// 
// 		for (int n = 0; n < _countof(PathMenuDef) ; n++)
// 		{
// 			m_pathMenuCmd.push_back(PathMenuDef[n]);
// 		}
// 
// 		TCHAR lpDriveString[MAX_PATH] = {0};
// 		GetLogicalDriveStrings(MAX_PATH,lpDriveString);
// 		int nCount  = 0;  
// 		TCHAR * pDrive = lpDriveString;  
// 		for(int nlen =_tcslen(pDrive); nlen == 3 ;nCount++)  
// 		{   
// 			//pDrive[2] = 0;
// 			DWORD dwMaxComLen = 0,dwFileSysFlag = 0;
// 			TCHAR szVolumeName[MAX_PATH] = {0};
// 			TCHAR fileSysBuf[MAX_PATH] = {0};
// 			
// 			UINT driveType = GetDriveType(pDrive);
// 
// 			
// 			do 
// 			{
// 				if ((DRIVE_FIXED != driveType && DRIVE_REMOVABLE != driveType))
// 				{
// 					break;;
// 				}
// 				if (!GetVolumeInformation(pDrive,szVolumeName,MAX_PATH * 2,NULL,&dwMaxComLen,&dwFileSysFlag,fileSysBuf,MAX_PATH))
// 				{
// 					break;
// 				}
// 
// 				menu_command menuCmd;
// 				menuCmd.strShow =  _tcslen(szVolumeName) ? szVolumeName : (driveType ==DRIVE_FIXED ?_T("本地磁盘") : _T("移动磁盘"));
// 				menuCmd.strShow.AppendFormat(_T("(%c:)"), pDrive[0]);
// 				menuCmd.strShow.MakeUpper();
// 				menuCmd.strValue = pDrive;
// 				m_pathMenuCmd.push_back(menuCmd);
// 
// 
// 			} while (FALSE);
// 			
// 			pDrive +=4;  
// 			nlen = _tcslen(pDrive);  
// 		}  
// 
// 		for (int nLoop = 0 ; nLoop < m_pathMenuCmd.size() ; nLoop++)
// 		{
// 			if (m_pathMenuCmd[nLoop].strShow.GetLength())
// 			{
// 				popMenu.AppendMenu(0, IDM_PATH_MENU_COMMAND_BEGIN + nLoop ,m_pathMenuCmd[nLoop].strShow);
// 			}
// 			else
// 			{
// 				popMenu.AppendMenu(MF_SEPARATOR, IDM_PATH_MENU_COMMAND_BEGIN + nLoop);
// 			}
// 		}
// 		
// 		DWORD dwPos = GetMessagePos();
// 		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
// 		popMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);
// 	}
// 
// 	if (msg.pSender->GetName() == _T("option_include_sub_dir_text") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_include_sub_dir")));
// 		if (pOption)
// 		{
// 			pOption->Selected(!pOption->IsSelected());
// 		}
// 	}
// 	if (msg.pSender->GetName() == _T("option_match_word_text") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_match_word")));
// 		if (pOption)
// 		{
// 			pOption->Selected(!pOption->IsSelected());
// 		}
// 	}
// 	if (msg.pSender->GetName() == _T("option_match_case_text") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_match_case")));
// 		if (pOption)
// 		{
// 			pOption->Selected(!pOption->IsSelected());
// 		}
// 	}
// 
// 	if (msg.pSender->GetName() == _T("option_match_Exclude_System_text") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_match_Exclude_System")));
// 		if (pOption)
// 		{
// 			pOption->Selected(!pOption->IsSelected());
// 		}
// 	}
// 	if (msg.pSender->GetName() == _T("option_match_Exclude_Hidden_text") && msg.sType == DUI_MSGTYPE_CLICK)
// 	{
// 		COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_match_Exclude_Hidden")));
// 		if (pOption)
// 		{
// 			pOption->Selected(!pOption->IsSelected());
// 		}
// 	}
// 
// 	if (msg.sType == DUI_MSGTYPE_KEYDOWN)
// 	{
// 		if (msg.wParam == VK_ESCAPE)
// 		{
// 			SendMessage(WM_CLOSE);
// 			return ;
// 		}
// 		
// 		if (VK_TAB == msg.wParam)
// 		{
// 			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
// 			RASSERT(pEdit,);
// 
// 			pEdit->SetFocus();
// 			pEdit->SetSelAll();
// 			return ;
// 		}
// 		
// 
// 
// 		if (msg.pSender->GetName() == _T("search_input_edit"))
// 		{
// 			switch(msg.wParam)
// 			{
// 			case VK_RETURN: break;
// 			case VK_DOWN:
// 				{
// 					COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_edit_select")));
// 					if (pOption)
// 					{
// 						CEditUI* pEdit = static_cast<CEditUI*>(msg.pSender);
// 						
// 						pOption->Selected(true);
// 					}
// 					break;
// 				}
// 			case VK_UP:
// 				{
// 				//	m_pResultList->SetFocus();
// 				}
// 				break;
// 			}
// 
// 		}
// 		/*
// 		if (msg.pSender == m_pResultList)
// 		{
// 			switch(msg.wParam)
// 			{
// 			case  'C':
// 				{
// 					CDuiString sFocus = m_pm.GetFocus()->GetName();
// 
// 					if ((sFocus == _T("virtual_list_result")) && (GetKeyState(VK_CONTROL) & 0x8000))
// 					{
// 						SendMessage(WM_COMMAND, ID_1002);
// 					}
// 					break;
// 				}//case  'C':
// 			
// 			case VK_RETURN:
// 				{
// 					SendMessage(WM_COMMAND, ID_1000);
// 					break;
// 				}//case VK_RETURN:
// 			
// 			
// 			
// 			}//switch(msg.wParam)
// 		
// 		}//if (msg.pSender == m_pResultList)
// 		*/
// 	}
// 
// 	if (msg.pSender->GetName() == _T("option_edit_select") &&  msg.sType == DUI_MSGTYPE_SELECTCHANGED)
// 	{
// 		CControlUI* pParent =  msg.pSender->GetParent();//static_cast<CControlUI*>(m_pm.FindControl(_T("option_edit_select_parent_horizontallayout")));
// 		if (pParent)
// 		{
// 			COptionUI* pSelect = static_cast<COptionUI*>(msg.pSender);
// 			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
// 			if (pEdit && pSelect)
// 			{
// 				if (pSelect->IsSelected())
// 				{
// //					m_SelectRecordWindow.Show(pEdit->GetPos(), pEdit->GetText());
// 				}
// 				else
// 				{
// 					//pEdit->SetFocus();
// //					m_SelectRecordWindow.Hide();
// 				}
// 			}
// 		}
// 	}
// 
// 	
// 	if (msg.pSender->GetName() == _T("select_window") )
// 	{
// 		/*
// 		if (msg.sType == DUI_MSGTYPE_KILLFOCUS)
// 		{
// 			COptionUI* pSelect = static_cast<COptionUI*>(m_pm.FindControl(_T("option_edit_select")));
// 			if (pSelect->IsSelected())
// 			{
// 				POINT cursorPont = {0};
// 				GetCursorPos(&cursorPont);
// 
// 				ScreenToClient(m_hWnd,&cursorPont);
// 				CControlUI* pFocus = m_pm.FindControl(cursorPont);
// 				if (pFocus !=  pSelect)
// 				{
// 					pSelect->Selected(false);
// 				}
// 			}
// 		}
// 		else if (msg.sType == DUI_MSGTYPE_ITEMACTIVATE)
// 		{
// 			
// 			LPCTSTR lpText = m_SelectRecordWindow.GetCulSel();
// 			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
// 			if (pEdit && lpText && _tclen(lpText))
// 			{
// 				pEdit->SetText(lpText);
// 				COptionUI* pSelect = static_cast<COptionUI*>(m_pm.FindControl(_T("option_edit_select")));
// 				if (pSelect)
// 				{
// 					pSelect->Selected(false);
// 				}
// 				
// 				PostMessage(SEARCH_RESEARCH_MSG,TRUE);
// 			}
// 			
// 		}
// 		else if (msg.sType == DUI_MSGTYPE_ITEMCLICK)
// 		{
// 			
// 			LPCTSTR lpText = m_SelectRecordWindow.GetCulSel();
// 			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
// 			if (pEdit && lpText && _tclen(lpText))
// 			{
// 				pEdit->SetText(lpText);
// 				PostMessage(SEARCH_RESEARCH_MSG,TRUE);
// 			}
// 			
// 		}
// 		*/
// 	}
// 	
// }


void CDuiWindow::OnClick(TNotifyUI& msg)
{
	CDuiString sCtrlName = msg.pSender->GetName();
	if( sCtrlName == _T("closebtn") )
	{
		Close();
		return; 
	}
	else if( sCtrlName == _T("minbtn"))
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
		return; 
	}
	else if( sCtrlName == _T("maxbtn"))
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
		return; 
	}
	else if( sCtrlName == _T("restorebtn"))
	{ 
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
		return; 
	}
	return;
}

LRESULT CDuiWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_pm.Init(m_hWnd);
	CDialogBuilder builder;
	CKYanBuildControl cb;
	CControlUI* pRoot = builder.Create(_T("skin.xml"), (UINT)0, &cb, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);
	


	InitMenu();

	
	return 0;
}

LRESULT CDuiWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	this->ShowWindow(false,false);
	
	
	this->ShowWindow(false,false);
	UTIL::com_ptr<IExit> pExit;
	m_pRot->GetObject(CLSID_MsEnv, __uuidof(IExit), (void**)&pExit);
	if (pExit)
	{
		pExit->NotifyExit();
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CDuiWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Close();
//	m_pResultList->UnInit();
	m_RClickPopMenu.DestroyMenu();
	bHandled = FALSE;
	return 0;
}

LRESULT CDuiWindow::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
	if (pEdit)
	{
		if (pEdit->IsFocused())
		{
			m_pm.SetFocusNeeded(pEdit);
			m_pm.SetFocus(pEdit);
			//pEdit->SetFocus();
		}
	}
	if( ::IsIconic(*this) ) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CDuiWindow::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//bHandled = TRUE;
	return 0;
}

LRESULT CDuiWindow::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

LRESULT CDuiWindow::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//bHandled = TRUE;
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	if( !::IsZoomed(*this) ) {
		RECT rcSizeBox = m_pm.GetSizeBox();
		if( pt.y < rcClient.top + rcSizeBox.top ) {
			if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
			if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
			return HTTOP;
		}
		else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
			if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
			if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
			return HTBOTTOM;
		}
		if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
		if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
	}

	RECT rcCaption = m_pm.GetCaptionRect();
	if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
			/*
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
				_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
				_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
				*/
				return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CDuiWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		RECT rc = { rcWnd.left, rcWnd.top + szRoundCorner.cx, rcWnd.right, rcWnd.bottom };
		HRGN hRgn1 = ::CreateRectRgnIndirect( &rc );
		HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom - szRoundCorner.cx, szRoundCorner.cx, szRoundCorner.cy);
		::CombineRgn( hRgn1, hRgn1, hRgn2, RGN_OR );
		::SetWindowRgn(*this, hRgn1, TRUE);
		::DeleteObject(hRgn1);
		::DeleteObject(hRgn2);
	}

	//bHandled = FALSE;
	return 0;
}
LRESULT CDuiWindow::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CDuiRect rcWork = oMonitor.rcWork;
	rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;
	lpMMI->ptMaxSize.x		= rcWork.right;
	lpMMI->ptMaxSize.y		= rcWork.bottom;
	return 0;
}

LRESULT CDuiWindow::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if( wParam == SC_CLOSE ) {
		//::PostQuitMessage(0L);
		OnClose(WM_CLOSE,wParam,lParam,bHandled);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed ) {
		if( !bZoomed ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(true);
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(false);
		}
	}
	return lRes;
}

/*
LRESULT CDuiWindow::OnCommand(UINT nID, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	WORD code = HIWORD(wParam);
	WORD id = LOWORD(wParam);
	HWND hWnd = (HWND)lParam;

	CString strLastSelectPath ;
	GetCurrentSelPath(strLastSelectPath);

	switch(id)
	{
	

	

	case ID_1002://复制文件
		{
			USES_CONVERSION;

			CopyFileToClipboard(W2A(strLastSelectPath.GetBuffer()));
			break;
		}

	
	
	
		 case ID_MENU_SEACH_DIR: //在当前目录下搜索
		 	{
				m_pPathEdit->SetCurrentPath(strLastSelectPath);
				//DoSearch(TRUE);
		 		//SearchInFolder(m_strLastSelectPath);
		 		break;
		 	}//case 1006: //在当前目录下搜索

		 default:
			 bHandled = FALSE;
	}

	
	return 0;
}
*/

LRESULT CDuiWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
	bHandled = FALSE;
	switch(wParam)
	{
	case VK_TAB:
		{
			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
			RASSERT(pEdit,0);

			pEdit->SetFocus();
			pEdit->SetSelAll();
			//pEdit->set
			break;
		}
	case VK_DOWN:
		{
			COptionUI* pOption = static_cast<COptionUI*>(m_pm.FindControl(_T("option_edit_select")));
			if (pOption)
			{
				

				pOption->Selected(true);
			}
			// _T("option_edit_select") &&  msg.sType == DUI_MSGTYPE_SELECTCHANGED
			//m_pm.SendNotify(m_pm.FindControl(_T("option_edit_select")), DUI_MSGTYPE_SELECTCHANGED);
			
			/*DUI_MSGTYPE_KEYDOWN*/
			break;
			
		}
	}
	return 0 ;
}

/*
LRESULT CDuiWindow::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	LPNMHDR lpNmhdr = (LPNMHDR)lParam;

	if (wParam == 100001)
	{
		return OnResultListNotify(uMsg,wParam, lParam, bHandled);
	}
	return 0;
}
*/

/*
LRESULT CDuiWindow::OnResultListNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	LPNMHDR lpNmhdr = (LPNMHDR)lParam;

	switch(lpNmhdr->code)
	{
	case LVN_GETDISPINFO:
		{
		

			

			break;
		}

		case NM_RCLICK:
			{
				

				break;
			}
		case NM_DBLCLK:
			{
				
			}

		case LVN_COLUMNCLICK:
			{
				break;
				//设置排序方式及箭头
// 				NMLISTVIEW* pLV = reinterpret_cast<NMLISTVIEW*>(lpNmhdr);
// 				int colIndex = pLV->iSubItem;
// 
// 				if (m_pResultList->m_SortCol==colIndex)
// 				{
// 					m_pResultList->m_Ascending = !m_pResultList->m_Ascending;
// 				}
// 				else
// 				{
// 					m_pResultList->m_SortCol = colIndex;
// 					m_pResultList->m_Ascending = (colIndex == LC_Stat_Name || colIndex == LC_Stat_Path) ? false : true;
// 				}
// 
// 				if (m_pResultList->SortColumn(m_pResultList->m_SortCol, m_pResultList->m_Ascending))
// 					m_pResultList->SetSortArrow(m_pResultList->m_SortCol, m_pResultList->m_Ascending);
// 				
// 
// 				bHandled = FALSE;
// 				int sortType = SORT_BY_NONE;
// 				if (m_pResultList->m_SortCol == LC_Stat_Name)
// 				{
// 					if (m_pResultList->m_Ascending)
// 						sortType = SORT_BY_NAME_ASCEND;
// 					else
// 						sortType = SORT_BY_NAME_DESCEND;
// 				}
// 
// 				if (m_pResultList->m_SortCol == LC_Stat_Path)
// 				{
// 					if (m_pResultList->m_Ascending)
// 						sortType = SORT_BY_PATH_ASCEND;
// 					else
// 						sortType = SORT_BY_PATH_DESCEND;
// 				}
// 
// 				if (m_pResultList->m_SortCol == LC_Stat_LastAccessTime)
// 				{
// 					if (m_pResultList->m_Ascending)
// 						sortType = SORT_BY_ACCESS_ASCEND;
// 					else
// 						sortType = SORT_BY_ACCESS_DESCEND;
// 				}
// 
// 				if (m_pResultList->m_SortCol == LC_Stat_Size)
// 				{
// 					if (m_pResultList->m_Ascending)
// 						sortType = SORT_BY_SIZE_ASCEND;
// 					else
// 						sortType = SORT_BY_SIZE_DESCEND;
// 				}
// 
// 				if(sortType != SORT_BY_NONE)
// 				{
// 					m_pFileSearch->Sort(sortType);
// 				}
				break;
			}
	}
	return 0;
}
*/
/*

LRESULT CDuiWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ) {
	//case WM_ADDLISTITEM:   lRes = OnAddListItem(uMsg, wParam, lParam, bHandled); break;
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	case WM_COMMAND:	   lRes = OnCommand(uMsg, wParam, lParam, bHandled); break;
	case WM_KEYDOWN:	   lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
	case WM_NOTIFY:		   lRes = OnNotify(uMsg, wParam, lParam, bHandled); break;
	case WM_QUERYENDSESSION:
		{
			UTIL::com_ptr<IExit> pExit;
			m_pRot->GetObject(CLSID_MsEnv , re_uuidof(IExit) , (void**)&pExit);
			if (pExit)
			{
				pExit->NotifyExit();
			}
			return 1;
		}
	case SEARCH_RESULT_CHANGE_MSG:
		{
			bHandled = FALSE;
			lRes = OnSearchResultChange(wParam, lParam);
			break;
		}
	case  SEARCH_TOTAL_COUNT_MSG:
		{
			bHandled = FALSE;
			lRes = OnSearchTotalCount(wParam, lParam); 
			break;
		}
	case SEARCH_RESEARCH_MSG:
		{
			bHandled = FALSE;
			lRes = OnSearchRefresh(wParam);
			break;
		}

	case  SEARCH_OBJECT_INIT_MSG:
		{
			bHandled = FALSE;
			if (m_pFileSearch)
			{
				WCHAR cDosDevice = LOWORD(wParam);
				ObjectInitNotify initNotify = (ObjectInitNotify) HIWORD(wParam);
				lRes = OnSearchObjectInit(cDosDevice, initNotify, lParam);
			}
			break;
		}
	case SORT_FILE_SEARCH_MSG:
		{
			WCHAR cDosDevice = wParam;
			if (lParam == SearchObjectSort_Begin)
			{
				CDeviceMap::iterator it = m_deviceMap.find(cDosDevice);
				if (it != m_deviceMap.end())
				{
					it->second.isSortCompete = FALSE;
				}

				m_pResultList->SetVisible(false);
				CHorizontalLayoutUI* pGif = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("gif_waiting_ctrl")));
				pGif->SetVisible(true);

				CGifControlUI* p = static_cast<CGifControlUI*>(m_pm.FindControl(_T("gif_ctrl_control")));
				p->SetVisible(true);

				CHorizontalLayoutUI* pLable = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("result_info_lable")));
				pLable->SetVisible(false);
				pLable = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("searching_label")));
				pLable->SetVisible(true);

				CString strMsg;
				for (CDeviceMap::iterator it = m_deviceMap.begin();
					it != m_deviceMap.end(); it++)
				{
					if (!it->second.isSortCompete)
					{
						strMsg.AppendFormat(_T("%c "), it->first);
					}
				}


				CLabelUI* pResultStatic = static_cast<CLabelUI*>(m_pm.FindControl(_T("search_root_disk")));
				pResultStatic->SetText(strMsg);
			}
			else if (lParam == SearchObjectSort_End)
			{
				CDeviceMap::iterator it = m_deviceMap.find(cDosDevice);
				if (it != m_deviceMap.end())
				{
					it->second.isSortCompete = TRUE;
				}

				BOOL isAllComplete = TRUE;
				for (CDeviceMap::iterator it = m_deviceMap.begin();
					it != m_deviceMap.end(); it++)
				{
					if (!it->second.isSortCompete)
					{
						isAllComplete = FALSE;
						break;
					}
				}

				//显示提示
				CString strMsg;
				for (CDeviceMap::iterator it = m_deviceMap.begin();
					it != m_deviceMap.end(); it++)
				{
					if (!it->second.isSortCompete)
					{
						strMsg.AppendFormat(_T("%c "), it->first);
					}
				}


				CLabelUI* pResultStatic = static_cast<CLabelUI*>(m_pm.FindControl(_T("search_root_disk")));
				pResultStatic->SetText(strMsg);

				if (isAllComplete)
				{
					m_pResultList->SetVisible(true);
					CHorizontalLayoutUI* pGif = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("gif_waiting_ctrl")));
					pGif->SetVisible(false);

					CControlUI* p = m_pm.FindControl(_T("gif_ctrl_control"));
					p->SetVisible(false);

					CHorizontalLayoutUI* pLable = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("result_info_lable")));
					pLable->SetVisible(true);
					pLable = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("searching_label")));
					pLable->SetVisible(false);

					 PostMessage(SEARCH_RESEARCH_MSG, TRUE, 0);
				}
				
			}

			break;
		}
	
	case  WM_DRAWITEM:
		{
			bHandled = FALSE;
			m_RClickPopMenu.OnDrawItem((UINT) wParam, (LPDRAWITEMSTRUCT) lParam);
			break;
		}
	case  WM_MEASUREITEM:
		{
			bHandled = FALSE;
			m_RClickPopMenu.OnMeasureItem((UINT) wParam, (LPMEASUREITEMSTRUCT) lParam);
			break;
		}
	case  WM_DEVICECHANGE:
		{
			bHandled = FALSE;
// 			if (m_pFileSearch)
// 			{
// 				lRes = m_pFileSearch->OnWndMessage(uMsg, wParam, lParam, bHandled);
// 			}

			break;
		}
	default:
		bHandled = FALSE;
	}

	
	
	if( bHandled ) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);

	
}
*/



//////////////////////////////////////////////////////////////////////////
//



 




HRESULT CDuiWindow::InitMenu()
{
	m_RClickPopMenu.LoadMenu(IDR_MENU_RCLICK);
	InitMenu(m_RClickPopMenu);

	return S_OK;
}

HRESULT CDuiWindow::InitMenu(CSkinMenu& skinMenu)
{
	skinMenu.SetBgPic(_T("Menu\\menu_left_bg.png"), _T("Menu\\menu_right_bg.png"));
	skinMenu.SetSelectedPic(_T("Menu\\menu_selected.png"));
	skinMenu.SetSepartorPic(_T("Menu\\menu_separtor.png"));
	skinMenu.SetArrowPic(_T("Menu\\menu_arrow.png"));
	skinMenu.SetCheckPic(_T("Menu\\menu_check.png"));
	skinMenu.SetTextColor(RGB(0, 20, 35));
	skinMenu.SetSelTextColor(RGB(254, 254, 254));
	return S_OK;
}

/*

*/

/*

*/

/*

*/

BOOL CDuiWindow::ShowActiveWindow()
{
	//this->ShowWindow();
	BOOL isZoom = ::IsZoomed(*this);

	//CreateDiskSearch();
	this->ShowWindow();
	if (isZoom)
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
	}
	
	::SetForegroundWindow(m_hWnd);
	::SetActiveWindow(m_hWnd);

	CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input_edit")));
	RASSERT(pEdit, FALSE);

	//pEdit->SetFocus();
	//pEdit->SetSelAll();
	return TRUE;
}






HRESULT CDuiWindow::OnShellContextMenuComX3Message( const CLIENTINFO& clientInfo, MESSAGEINFO* pMessageInfo, IMessage*pMessage )
{
	/*
	int n = sizeof(comx3_shell_contextmenu_info);
	if (!pMessageInfo->pInData || pMessageInfo->nInCch != sizeof(comx3_shell_contextmenu_info))
	{
		return S_OK;
	}

	comx3_shell_contextmenu_info cmd;
	CopyMemory(&cmd, pMessageInfo->pInData, sizeof(comx3_shell_contextmenu_info));

	CParseCommand ParseCmd;
	USES_CONVERSION;
	ParseCmd.ParseCmd(A2W(cmd.strFilePath));
	LPCTSTR lpszFilterPath = ParseCmd.GetArg(_T("filter"));

	m_strFilterPathFormNpc = lpszFilterPath;

	//CreateTableItem(lpszFilterPath);
	PostMessage(NPC_FILTER_PATH_MESSAGE);
// 	if (lpszFilterPath && _tcslen(lpszFilterPath))
// 	{
// 		m_pPathEdit->SetCurrentPath(lpszFilterPath);
// 		//DoSearch(TRUE);
// 	}
*/
	
	return S_OK;
}


LRESULT CDuiWindow::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	m_RClickPopMenu.OnDrawItem((UINT) wParam, (LPDRAWITEMSTRUCT) lParam);
	return 0;
}

LRESULT CDuiWindow::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	m_RClickPopMenu.OnMeasureItem((UINT) wParam, (LPMEASUREITEMSTRUCT) lParam);
	return 0;
}

LRESULT CDuiWindow::OnSelectPathBtnClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	WTL::CFolderDialog folderDlg;
	RASSERT(IDOK == folderDlg.DoModal(m_hWnd), 0);
	CString strPath = folderDlg.GetFolderPath();
		
// 	CPathFilter pathFilter;
// 	RASSERT(pathFilter.Init(m_pRot), FALSE);

// 	CPathFilVector folder;
// 	pathFilter.GetAll(folder);

// 	PATH_FILTER filter;
// 	filter.strPath = strPath.GetBuffer();
// 	filter.nLeave = 0;
// 
// 	folder.insert(folder.begin(), filter);
// 	if (pathFilter.DeleteAll())
// 	{
// 		pathFilter.Add(folder);
// 	}

	CreateTableItem(strPath);

	return 0;
}

LRESULT CDuiWindow::OnMenuVisitWebsite(WORD id, WORD code)
{
	LPCWSTR lpszWebSite = GetEnvParamString("website");
	if (lpszWebSite && wcslen(lpszWebSite))
	{
		try
		{
			ShellExecute(this->GetHWND(), _T("open"), lpszWebSite,NULL, NULL, SW_SHOWNORMAL);
		}
		catch(...){}
	}

	return 0;
}

LRESULT CDuiWindow::OnMenuRestoreWindow(WORD id, WORD code)
{
	ShowActiveWindow();
	return 0;
}

LRESULT CDuiWindow::OnMenuExit(WORD id, WORD code)
{
	
	this->ShowWindow(false,false);
	UTIL::com_ptr<IExit> pExit;
	m_pRot->GetObject(CLSID_MsEnv, __uuidof(IExit), (void**)&pExit);
	if (pExit)
	{
		pExit->NotifyExit();
	}
	return 0;
}

/*

*/


LRESULT CDuiWindow::OnMenuReBuildIndex(WORD id, WORD code)
{
	/*
	UTIL::com_ptr<IFileSearchSrv> pSearchSrv;
	m_pRot->GetObject(CLSID_FileSearchSrv, re_uuidof(IFileSearchSrv), (void**)&pSearchSrv);
	RASSERT(pSearchSrv, 0);
	pSearchSrv->ReBuildIndex();
	*/
	return 0;
}

LRESULT CDuiWindow::OnTrayMenuSearhFolder(WORD id, WORD code)
{

	return 0;
}

LRESULT CDuiWindow::OnTrayMenuMonitorFileChange(WORD id, WORD code)
{
// 	if (!IsInstallLdrSrv())
// 	{
// 		CServiceOpt srvOpt;
// 		srvOpt.SetServiceName(_T("lockldr"));
// 		DWORD dwState = 0;
// 
// 		if(srvOpt.Init())
// 		{
// 			return 0;
// 		}
// 
// 		if (FAILED(srvOpt.QueryServiceStatus(dwState)))
// 		{
// 			srvOpt.Install(_T("C:\\Windows\\System32\\drivers\\lockldr.sys"), _T("lockldr"), SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START, _T("快眼文件变动监视服务"));
// 		}
// 
// 		if (dwState != SERVICE_RUNNING)
// 		{
// 			srvOpt.Start(FALSE);
// 		}
// 	}
	return 0;
}

STDMETHODIMP CDuiWindow::NotifyExit(bool* bExit /*= NULL*/)
{
	PostQuitMessage(0);
	return S_OK;
}


void  CDuiWindow::OnMinBtnClick(TNotifyUI& msg)
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
}

void  CDuiWindow::OnMaxBtnClick(TNotifyUI& msg)
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
}

void  CDuiWindow::OnRestoreBtnClick(TNotifyUI& msg)
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
}

void  CDuiWindow::OnCloseBtnClick(TNotifyUI& msg)
{
	//Close();
	ShowWindow(false,false);
}

void CDuiWindow::OnEditTextChange(TNotifyUI& msg)
{
}

void CDuiWindow::OnHeaderDbClick(TNotifyUI& msg)
{

}

void CDuiWindow::OnAddBtnClick(TNotifyUI&)
{
	CreateTableItem(_T(""));
}

