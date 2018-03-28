#include "stdafx.h"
#include "dlgUpdate.h"


DUI_BEGIN_MESSAGE_MAP(CDlgUpdate,WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT,OnWinInit)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelChange)
DUI_END_MESSAGE_MAP()


CDlgUpdate::CDlgUpdate()
{
}

CDlgUpdate::~CDlgUpdate()
{

}

void CDlgUpdate::OnWinInit(TNotifyUI& msg)
{
	m_opt = dynamic_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_no_tip")));
	if (m_opt)
	{
		m_opt->Selected(!NeedTip());
	}
}

void CDlgUpdate::OnClick(TNotifyUI& msg)
{
	CString strName = msg.pSender->GetName();
	if (strName == _T("closebtn"))
	{
		Close(0);
	}
	else if (strName == _T("btn_ok"))
	{
		Close(1);
		SetTip(!m_opt->IsSelected());
		SetAutoUpdate(1);
	}
	else if (strName == _T("btn_no"))
	{
		Close(0);
		SetTip(!m_opt->IsSelected());
		SetAutoUpdate(0);
	}
}

void CDlgUpdate::OnSelChange(TNotifyUI& msg)
{
	CString strName = msg.pSender->GetName();
	if (strName == _T("opt_no_tip"))
	{
		COptionUI* opt = dynamic_cast<COptionUI*>(msg.pSender);
		SetTip(!opt->IsSelected());
	}
}

bool CDlgUpdate::NeedTip()
{
	BOOL bRet = GetPrivateProfileInt(_T("LGPEINSTIMM"), _T("UpdateTip"), 1, CONFIG_HIDEPATH);

	return bRet;
}

void CDlgUpdate::SetTip(bool bTip)
{
	WritePrivateProfileString(_T("LGPEINSTIMM"), _T("UpdateTip"), bTip?_T("1"):_T("0"), CONFIG_HIDEPATH);
}

bool CDlgUpdate::AutoUpdate()
{
	BOOL bRet = GetPrivateProfileInt(_T("LGPEINSTIMM"), _T("AutoUpdate"), 1, CONFIG_HIDEPATH);

	return bRet;
}

void CDlgUpdate::SetAutoUpdate(bool bAuto)
{
	WritePrivateProfileString(_T("LGPEINSTIMM"), _T("AutoUpdate"), bAuto?_T("1"):_T("0"), CONFIG_HIDEPATH);
}
