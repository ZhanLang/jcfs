#pragma once

#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#include "CListCtrl_Sortable.h"

class CWTLVirtualList:
	public SWindow,
	public CListCtrl_Sortable
{
public:

	SOUI_CLASS_NAME(CWTLVirtualList,L"WTLVirtualList");

	CWTLVirtualList(void);
	~CWTLVirtualList(void);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_SIZE(OnSize)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SHOWWINDOW(OnShowWindow)
	SOUI_MSG_MAP_END()
	
	BEGIN_MSG_MAP_EX(CWTLVirtualList)
		MSG_WM_CREATE(OnCreate)
	END_MSG_MAP()

	//soui
	int	OnCreate(void*);
	void OnSize(UINT , SOUI::CSize size);
	void OnShowWindow(BOOL bShow, UINT nStatus);

	LRESULT OnCreate(LPCREATESTRUCT lParam);
	virtual void SetVisible(bool bVisible = true);

	//////////////////////////////////////////////////////////////////////////
	 void CWTLVirtualList::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	
};

