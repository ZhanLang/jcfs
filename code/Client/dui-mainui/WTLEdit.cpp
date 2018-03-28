#include "StdAfx.h"
#include "WTLEdit.h"


CWTLEdit::CWTLEdit(void)
{
}


CWTLEdit::~CWTLEdit(void)
{
}

void CWTLEdit::DoInit()
{
	HWND hParentWnd = m_pManager->GetPaintWindow();
	CSkinEdit::Create(hParentWnd,m_rcPaint, NULL, WS_CHILD|WS_VISIBLE, NULL, 200);
	this->ShowWindow(SW_SHOW);
}

LPCTSTR CWTLEdit::GetClass() const
{
	return _T("WTLEditUI");
}

LPVOID CWTLEdit::GetInterface(LPCTSTR pstrName)
{
	return CControlUI::GetInterface(pstrName);
}

UINT CWTLEdit::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

void CWTLEdit::SetPos(RECT rc)
{
	if(IsVisible())
	{
		this->MoveWindow(&rc,TRUE);
	}

	CControlUI::SetPos(rc);
}


void CWTLEdit::SetVisible(bool bVisible/* = true*/)
{
	if (this->IsWindow())
	{
		this->ShowWindow( bVisible ? SW_SHOW : SW_HIDE);
	}

	CControlUI::SetVisible(bVisible);
}

void CWTLEdit::DoEvent(DuiLib::TEventUI& event)
{

}

/*
BOOL SetBgNormalPic(LPCTSTR lpszFileName, RECT * lpNinePart = NULL);
BOOL SetBgHotPic(LPCTSTR lpszFileName, RECT * lpNinePart = NULL);
BOOL SetIconPic(LPCTSTR lpszFileName);
void SetTransparent(BOOL bTransparent, HDC hBgDC);
void SetDefaultText(LPCTSTR lpszText);
BOOL IsDefaultText();
void SetDefaultTextMode(BOOL bIsDefText);
void SetMarginsEx(int nLeft, int nTop, int nRight, int nBottom);	// 设置上下左右边距函数
void SetMultiLine(BOOL bMultiLine = TRUE);
BOOL SubclassWindow(HWND hWnd);
*/
void CWTLEdit::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{

}

//////////////////////////////////////////////////////////////////////////
LRESULT CWTLEdit::SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	return 0;
}

void	CWTLEdit::SetFocus()
{

}