#include "stdafx.h"
#include "UIAnimatRotate.h"
#include "dlgCheck.h"


DUI_BEGIN_MESSAGE_MAP(CDlgCheckUpdate,WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT,OnWinInit)
DUI_END_MESSAGE_MAP()

CDlgCheckUpdate::CDlgCheckUpdate()
{

}

CDlgCheckUpdate::~CDlgCheckUpdate()
{

}

CControlUI* CDlgCheckUpdate::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("AnimationRotate")) == 0)
	{
		return new CAnimatRotate;
	}
	return NULL;
}

void CDlgCheckUpdate::OnFinalMessage(HWND hWnd)
{
	delete this;
}

LRESULT CDlgCheckUpdate::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;

	switch(uMsg)
	{
	case WM_EX_UPDATE:
		if (wParam == 0)
		{
			if (lParam == 0)
			{
				m_tab->SelectItem(PAGE_NEED);
			}
			else if (lParam == 1)
			{
				m_tab->SelectItem(PAGE_NOTNEED);
			}
			else if (lParam == 2)
			{
				m_tab->SelectItem(PAGE_ERR);
			}
			else
			{
				m_tab->SelectItem(PAGE_ERR);
			}
		}
		else if (wParam == 100)
		{
			Close(0);
		}
		break;
	}

	return 0;
}

void CDlgCheckUpdate::OnClick(TNotifyUI& msg)
{
	CString strName = msg.pSender->GetName();
	if (strName == _T("closebtn"))
	{
		g_wndIMM->PostMessage(WM_EX_DO_UPDATE, 1);
		Close(0);
	}
	else if (strName == _T("btn_update"))
	{
		m_update.StartUpdate(true);
		ShowWindow(false);
	}
	else if (strName == _T("btn_retry"))
	{
		m_tab->SelectItem(PAGE_CHECK);
		m_update.StartCheck(m_hWnd);
	}
}

void CDlgCheckUpdate::OnWinInit(TNotifyUI& msg)
{
	m_tab = dynamic_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_msg")));

	m_update.StartCheck(m_hWnd);
}
