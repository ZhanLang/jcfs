#include "stdafx.h"
#include "WTLVirtualList.h"



LPCTSTR CWTLVirtualList::GetWindowClassName() const
{
	return _T("V_LIST_WND");
}

LPCTSTR CWTLVirtualList::GetSuperClassName() const
{
	return WC_LISTVIEW;
}


LPCTSTR CWTLVirtualList::GetClass() const
{
	return _T("WTLVirtualListoUI");
}


LPVOID CWTLVirtualList::GetInterface(LPCTSTR pstrName)
{
	  return CControlUI::GetInterface(pstrName);
}

void CWTLVirtualList::DoInit()
{
	
	HWND hParentWnd = m_pManager->GetPaintWindow();
	Create(hParentWnd, 0, _T(""), 
		WS_CHILDWINDOW 
		| WS_VISIBLE
		| LVS_REPORT
		//| WS_CLIPSIBLINGS 
	//	| WS_CLIPCHILDREN 
		| WS_HSCROLL 
		//| WS_MAXIMIZEBOX
		//| LVS_ALIGNLEFT 
		| LVS_OWNERDATA
		//|LVS_SHOWSELALWAYS 
	//	| LVS_OWNERDRAWFIXED
		, 
		WS_EX_LEFT
		
		//| WS_EX_LTRREADING
		//|WS_EX_RIGHTSCROLLBAR
		/*|WS_EX_CONTROLPARENT*/
		, 100001);

	CListCtrl_Sortable::OnCreate(NULL);
	this->SetVisible(IsVisible());
	
}
UINT CWTLVirtualList::GetControlFlags() const
{
	 return UIFLAG_TABSTOP;
}

 void CWTLVirtualList::SetPos(RECT rc)
 {
	 if(::IsWindow(m_hWnd) && IsVisible())
	 {
		// ::SetWindowPos(m_hWnd,HWND_BOTTOM,rc.left , rc.top, rc.right, rc.bottom, SWP_NOMOVE);
		 this->MoveWindow(&rc, TRUE);
	 }
	 /*
	 if(IsVisible())
	 {
		  this->MoveWindow(&rc,TRUE);

		  //特殊处理了
		  int nAllColumn = 0;   //总共的宽度
		  int nHeadWidth = 0;	//从大小开始
		  for (int n = 0 ; n < GetColumnCount() ; n++)
		  {
			  int nWidth = GetColumnWidth(n);
			  nAllColumn += nWidth;
			  if (n > 1)
			  {
				  nHeadWidth += nWidth;
			  }
		  }

		  CRect rect = rc;
		  int nSubWidth = rect.Width() - nAllColumn - 15;
		  SetColumnWidth(0, GetColumnWidth(0) + (nSubWidth * 0.25));
		  SetColumnWidth(1, GetColumnWidth(1) + (nSubWidth * 0.75));
	 }
	 */
	 CControlUI::SetPos(rc);
	
 }


 LRESULT CWTLVirtualList::OnCreate(LPCREATESTRUCT lParam)
 {
	 return CListCtrl_Sortable::OnCreate(lParam);
 }

 void CWTLVirtualList::SetVisible(bool bVisible /*= true*/)
 {
 	 if (::IsWindow(m_hWnd))
 	 {
		 this->ShowWindow(bVisible?SW_SHOW:SW_HIDE);
 	 }
 		 
	 CControlUI::SetVisible(bVisible);
 }

 void CWTLVirtualList::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
 {
	 CControlUI::SetAttribute(pstrName, pstrValue);

	 if (::IsWindow(m_hWnd) && _tcsicmp(_T("visible"), pstrName) == 0)
	 {
		 this->ShowWindow(CControlUI::IsVisible() ? SW_SHOW : SW_HIDE);
	 }
 }

 LRESULT CWTLVirtualList::SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
 {
	 /*
	 if (hWnd == CSkinVitrualListCtrl::GetParent())
	 {
		 if (Msg == WM_NOTIFY)
		 {
			 LPNMHDR pstNmhdr = (LPNMHDR)lParam;
			 if (pstNmhdr->code == NM_DBLCLK)
			 {
				 m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMDBCLICK);
			 }
			 else if (pstNmhdr->code == NM_RCLICK)
			 {
				 m_pManager->SendNotify(this, DUI_MSGTYPE_RBUTTONUP);
			 }
		 }
		 else if (Msg == WM_KEYDOWN)
		 {
			 GetManager()->SendNotify(this, DUI_MSGTYPE_KEYDOWN, wParam, lParam);
		 }
		
	 }
	 else
	 {
		  return CSkinVitrualListCtrl::SendMessage(hWnd, Msg, wParam, lParam);
	 }
	 */
	return 0;
 }

void CWTLVirtualList::SetFocus()
{
	m_pManager->SetFocus(this);
	//CSkinVitrualListCtrl::SetFocus();
}

void CWTLVirtualList::DoEvent(DuiLib::TEventUI& event)
{
	INT A = 0;
}
CWTLVirtualList::CWTLVirtualList(void)
{
}


CWTLVirtualList::~CWTLVirtualList(void)
{
	//UnInit();
}


// LRESULT CWTLVirtualList::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
// 	switch(uMsg)
// 	{
// 	case WM_NOTIFY:
// 		
// 			HANDLE_WM_NOTIFY(m_hWnd,wParam,lParam, OnNotify);
// 			
// 		
// 
// 		
// 	//default:
// 		//return __super::de(uMsg, wParam, lParam);
// 	}
// 	
// 	return 0;
// }

LRESULT CWTLVirtualList::OnNotify(int idCtrl, LPNMHDR lParam)
{
	switch(idCtrl)
	{
	case LVN_GETDISPINFO:
		//
		OutputDebugString(_T("aaaaaaaaaa"));
	}
	return TRUE;
}