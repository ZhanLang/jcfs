#include "StdAfx.h"
#include "DuiWindow.h"
#include <msapi/mswinapi.h>
// DUI_BEGIN_MESSAGE_MAP(CDuiWindow,WindowImplBase)
// 	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT,OnInitialization)
// 	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CDuiWindow::OnClick)
// DUI_END_MESSAGE_MAP()

CDuiWindow::CDuiWindow(void)
{

}


CDuiWindow::~CDuiWindow(void)
{
	CPaintManagerUI::Term();
}


STDMETHODIMP CDuiWindow::Init(void* pParam)
{
	CPaintManagerUI::SetInstance(g_hinstance);

	m_pSetupWizard = (ISetupWizard*)pParam;
	
	if (m_pSetupWizard->IsDebug())
	{
		TCHAR szTemp[MAX_PATH] = { 0 };
		msapi::GetCurrentPath(szTemp, MAX_PATH);
		CDuiString path = szTemp;
		path.Append(_T("\\setup_skin\\"));

		CPaintManagerUI::SetResourceType(UILIB_FILE);
		CPaintManagerUI::SetResourcePath(path);
	}
	else
	{
		CString strZipUI;
		GetTempPath(MAX_PATH, strZipUI.GetBufferSetLength(MAX_PATH));
		strZipUI.ReleaseBuffer();
		strZipUI.Append(_T("\\JCDeskSetup.zip"));

		msapi::ExportResourceToFile(g_hinstance, IDR_ZIPRES1, _T("ZIPRES"), strZipUI);

		CPaintManagerUI::SetResourceZip(strZipUI);
		CPaintManagerUI::SetResourceType(UILIB_ZIP);
	}

	m_pSetupWizard->SetWizardNotify(static_cast<ISetupWizardNotifty*>(this));

	CString strProdName = m_pSetupWizard->GetFriendProdName();
	strProdName += _T("-°²×°³ÌÐò");

	WindowImplBase::Create(NULL, strProdName, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 650, 434);
	this->SetIcon(IDI_NEWSETUP);
	CenterWindow();
	return TRUE;
}

STDMETHODIMP CDuiWindow::Uninit()
{
	CSubThread::StopThread(TRUE);
	return S_OK;
}

STDMETHODIMP CDuiWindow::Start()
{
	MSG msg = { 0 };

	BOOL bRet;

	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			return -1;
		}
		else
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}

	return 0;
}

STDMETHODIMP CDuiWindow::Stop()
{
	return S_OK;
}

DuiLib::CDuiString CDuiWindow::GetSkinFile()
{
	return _T("setup.xml");
}

LPCTSTR CDuiWindow::GetWindowClassName(void) const
{
	return m_pSetupWizard->GetProductName();
}




LRESULT CDuiWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
	bHandled = TRUE;
	return 0;
}

LRESULT CDuiWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	this->ShowWindow(false);
	PostQuitMessage(0);
	bHandled = TRUE;
	return 0;
}


STDMETHODIMP_(VOID) CDuiWindow::OnSetProgress(INT nProgress)
{

	PostMessage(WMMSG_SET_PROG, nProgress);
	
}

STDMETHODIMP_(BOOL) CDuiWindow::GetControlCheckState(LPCTSTR lpszControlName)
{
	CCheckBoxUI* pCheckBoxUI = static_cast<CCheckBoxUI*>(m_pm.FindControl(lpszControlName));
	if (pCheckBoxUI)
	{
		return  pCheckBoxUI->IsSelected();
	}
	return TRUE;
}

STDMETHODIMP_(CString) CDuiWindow::GetProgramPosition()
{
	CEditUI* pInstallPathEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("input_floder")));
	if (pInstallPathEdit)
	{
	 	return pInstallPathEdit->GetText().GetData();
	}
	return _T("");
}

STDMETHODIMP_(VOID) CDuiWindow::OnError(ErrorType errorType)
{
	PostMessage(WMMSG_SET_PAGE, 3);
}

STDMETHODIMP_(VOID) CDuiWindow::OnFinesh()
{
	PostMessage(WMMSG_SET_PAGE, 2);
}

DuiLib::CDuiString CDuiWindow::GetZIPFileName() const
{
	CDuiString fileName = m_pSetupWizard->GetTempCheckOutPath();
	fileName += _T("\\setup\\skin.zip");
	return fileName;
}

LPCTSTR CDuiWindow::GetResourceID() const
{
	 return MAKEINTRESOURCE(IDR_ZIPRES1);
}

void CDuiWindow::OnInitialization(TNotifyUI& msg)
{

}

VOID CDuiWindow::OnClick(TNotifyUI& msg)
{
	

	if ( _tcsicmp(msg.pSender->GetName(), _T("select_floder_brn")) == 0)
	{
		WTL::CFolderDialog folderDlg(m_hWnd);
		if(IDOK == folderDlg.DoModal(m_hWnd))
		{
			CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("input_floder")));
			if (pEdit)
			{
				CString strPath = folderDlg.GetFolderPath();
				CString strTemp = strPath;
				strTemp.MakeLower();

				CString strProdName = m_pSetupWizard->GetProductName();
				strProdName.MakeLower();
				if ( strTemp.Find(strProdName) == -1)
				{
					strPath.AppendFormat(_T("\\%s"), m_pSetupWizard->GetProductName());
				}

				strPath.Replace(_T("\\\\"), _T("\\"));
				pEdit->SetText(strPath);
			}
		}
	}

	if (_tcsicmp(_T("install_button"), msg.pSender->GetName()) == 0)
	{
		CTabLayoutUI* pTable = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
		if ( pTable )
		{
			pTable->SelectItem(1);
		}
		
		CSubThread::StartThread();
	}

	if (_tcsicmp(_T("install_button_finesh"), msg.pSender->GetName()) == 0 || 
		_tcsicmp(_T("closebtn"), msg.pSender->GetName()) == 0)
	{
		this->Close();
		PostQuitMessage(0);
	}

	if (_tcsicmp(_T("install_button_run"), msg.pSender->GetName()) == 0 )
	{
		this->Close();
		;
		;

		CDuiString sPath ;sPath.Format(_T("%s\\%s.exe"), 
			m_pSetupWizard->GetProgramPosition(),
			m_pSetupWizard->GetProductName());

		//msapi::Execute(sPath, sPath, FALSE, TRUE, 0);
		//PostQuitMessage(0);
	}


	WindowImplBase::OnClick(msg);
}

void CDuiWindow::InitWindow()
{
	CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("input_floder")));
	if ( pEdit )
	{
		pEdit->SetText(m_pSetupWizard->GetProgramPosition());
	}
}

HRESULT CDuiWindow::Run()
{
	if (m_pSetupWizard)
	{
		CControlUI* pControl = m_pm.FindControl(_T("closebtn"));
		if ( pControl )
		{
			pControl->SetEnabled(false);
		}
		m_pSetupWizard->DoSetup();
	}

	return 0;
}

STDMETHODIMP_(VOID) CDuiWindow::OnExtractPercent(INT nProgress)
{
	OnSetProgress(nProgress);
	return;
	CProgressUI* pControl = static_cast<CProgressUI*>(m_pm.FindControl(_T("update_percent")));
	if (pControl)
	{
		pControl->SetVisible(true);
		pControl->SetValue(nProgress);
	}
}

STDMETHODIMP_(VOID) CDuiWindow::OnStep(StepType step)
{
	CTabLayoutUI* pTable = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
	RASSERTV(pTable);

	switch( step )
	{
	case StepType_UnCompress:
	case StepType_DoScript:
		PostMessage(WMMSG_SET_PAGE, 1);
		break;
	}
}

LRESULT CDuiWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch( uMsg )
	{
	case WMMSG_SET_PAGE:
		{
			CTabLayoutUI* pTable = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
			if(pTable)
				for(int nLoop = 0 ; nLoop < 4 ; nLoop++)
					pTable->SelectItem(wParam);

			RECT R = m_pm.GetRoot()->GetPos();
			m_pm.Invalidate(R);
		}
		

		break;

	case WMMSG_SET_PROG:
		{
			CProgressUI* pControl = static_cast<CProgressUI*>(m_pm.FindControl(_T("update_percent")));
			if (pControl)
			{
				pControl->SetVisible(true);
				pControl->SetValue(wParam);
			}

			CLabelUI* pLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("inst_lbl_per")));
			if ( pLabel )
			{
				CDuiString strTmp;
				strTmp.Format(_T("%d"), wParam);
				pLabel->SetVisible(true);
				pLabel->SetText(strTmp);
				pLabel->SetVisible(true);
				CDuiRect rt;
				rt.top = 397;
				rt.bottom = 416;
				rt.left = 10;
				rt.left += ( 600 * wParam) / 100;
				rt.right = rt.left + 24;
				pLabel->SetPos(rt);
			}
		}
		

		break;
	}

	return 0;
}




