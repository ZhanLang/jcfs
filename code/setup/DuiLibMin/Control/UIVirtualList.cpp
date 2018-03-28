#include "stdafx.h"
#include "duilib/Control/UIVirtualList.h"

namespace DuiLib{;



CVirtualListUI::CVirtualListUI()
	:m_pListHeadUI(NULL)
{

}

LPCTSTR CVirtualListUI::GetClass() const
{
	  return _T("VirtualList");
}

UINT CVirtualListUI::GetControlFlags() const
{
	 return UIFLAG_TABSTOP;
}

LPVOID CVirtualListUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("IContainer")) == 0 ) return static_cast<IContainerUI*>(this);
	return CContainerUI::GetInterface(pstrName);
}

// CControlUI* CVirtualListUI::GetItemAt(int iIndex) const
// {
// 	return NULL;
// }
// 
// int CVirtualListUI::GetItemIndex(CControlUI* pControl) const
// {
// 	return -1;
// }
// bool CVirtualListUI::SetItemIndex(CControlUI* pControl, int iIndex)
// {
// 	return false;
// }
// 
// int CVirtualListUI::GetCount() const 
// {
// 	return 0;
// }
// 
bool CVirtualListUI::Add(CControlUI* pControl)
{
	if(_tcscmp(pControl->GetClass(), _T("ListHeaderUI")) == 0 )
	{
		ASSERT(!m_pListHeadUI);
		m_pListHeadUI = static_cast<CListHeaderUI*>(pControl);
		CVerticalLayoutUI::Add(pControl);
	}
	return true;
}

// bool CVirtualListUI::AddAt(CControlUI* pControl, int iIndex)
// {
// 	return false;
// }
// 
// bool CVirtualListUI::Remove(CControlUI* pControl)
// {
// 	return false;
// }
// bool CVirtualListUI::RemoveAt(int iIndex)
// {
// 	return false;
// }
// void CVirtualListUI::RemoveAll()
// {
// 	
// }

// void CVirtualListUI::DoPaint(HDC hDC, const RECT& rcPaint)
// {
// 	if (m_pListHeadUI)
// 	{
// 		m_pListHeadUI->DoPaint(hDC,rcPaint);
// 	}
// }

// void CVirtualListUI::PaintBkColor(HDC hDC)
// {
// 	if (m_pListHeadUI)
// 	{
// 		m_pListHeadUI->PaintBkColor(hDC);
// 	}
// }
// void CVirtualListUI::PaintBkImage(HDC hDC)
// {
// 	if (m_pListHeadUI)
// 	{
// 		m_pListHeadUI->PaintBkImage(hDC);
// 	}
// }
// 
// void CVirtualListUI::PaintStatusImage(HDC hDC)
// {
// 	if (m_pListHeadUI)
// 	{
// 		m_pListHeadUI->PaintStatusImage(hDC);
// 	}
// }
// 
// void CVirtualListUI::PaintText(HDC hDC)
// {
// 	if (m_pListHeadUI)
// 	{
// 		m_pListHeadUI->PaintText(hDC);
// 	}
// }
// 
// void CVirtualListUI::PaintBorder(HDC hDC)
// {
// 	if (m_pListHeadUI)
// 	{
// 		m_pListHeadUI->PaintBorder(hDC);
// 	}
// }

}//namespace DuiLib{;}