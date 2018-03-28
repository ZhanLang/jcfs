#include "stdafx.h"
#include "dlgExit.h"


DUI_BEGIN_MESSAGE_MAP(CDlgExit,WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()


CDlgExit::CDlgExit()
{
	m_bExit = false;
}

CDlgExit::~CDlgExit()
{

}

void CDlgExit::OnClick(TNotifyUI& msg)
{
	CString strName = msg.pSender->GetName();
	if (strName == _T("closebtn"))
	{
		Close(0);
	}
	else if (strName == _T("btn_ok"))
	{
		Close(1);
	}
	else if (strName == _T("opt_exit"))
	{
		m_bExit = true;
	}
	else if (strName == _T("opt_hide"))
	{
		m_bExit = false;
	}
}
