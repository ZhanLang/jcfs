#include "stdafx.h"
#include "WTLVirtualList.h"


 LRESULT CWTLVirtualList::OnCreate(LPCREATESTRUCT lParam)
 {
	 //return 
	 HWND hParentWnd = SWindow::GetContainer()->GetHostHwnd();
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
		 //|WS_EX_CONTROLPARENT
		 , IDC_RESULT_LIST);

	 CListCtrl_Sortable::OnCreate(NULL);
	 this->SetVisible(SWindow::IsVisible());
	 return 0;
 }

 int CWTLVirtualList::OnCreate(void*)
 {
	 return 0;
 }

 void CWTLVirtualList::OnSize(UINT, SOUI::CSize size)
 {
	 SWindow::OnSize(0, size);
	 if(::IsWindow(m_hWnd) && IsVisible())
	 {
		 this->MoveWindow(&SWindow::GetClientRect(), TRUE);
	 }

 }

 void CWTLVirtualList::OnShowWindow(BOOL bShow, UINT nStatus)
 {
	 if (::IsWindow(m_hWnd))
	 {
		 this->ShowWindow(bShow?SW_SHOW:SW_HIDE);
	 }
 }

 void CWTLVirtualList::SetVisible(bool bVisible /*= true*/)
 {
	 SWindow::SetVisible(bVisible);
 }

 void CWTLVirtualList::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
 {
	 if (::IsWindow(m_hWnd) && _tcsicmp(_T("visible"), pstrName) == 0)
	 {
		 this->ShowWindow(SWindow::IsVisible() ? SW_SHOW : SW_HIDE);
	 }
 }



CWTLVirtualList::CWTLVirtualList(void)
{
}


CWTLVirtualList::~CWTLVirtualList(void)
{
	//UnInit();
}
