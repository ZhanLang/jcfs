#pragma once


#include "SkinHeaderCtrl.h"

class CListCtrl_Sortable : public CListViewCtrl
{
public:
	bool m_Ascending;
	int  m_SortCol;

 	BEGIN_MSG_MAP_EX(CListCtrl_Sortable)

 		//NOTIFY_HANDLER(,OnNotify)
		
 		MSG_WM_CREATE(OnCreate)
		MSG_WM_DRAWITEM (OnDrawItem)
 	END_MSG_MAP()

	BOOL OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);
	void PreSubclassWindow();


	LRESULT OnCreate(LPCREATESTRUCT lParam);
	LRESULT OnDrawItem(WPARAM wParam, LPDRAWITEMSTRUCT drawItem);
	static LRESULT CALLBACK ListViewHeaderWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam);
public:
	CListCtrl_Sortable();
	~CListCtrl_Sortable();


	int GetColumnData(int col) const;
	void SetSortArrow(int col, bool ascending);
	bool IsAscending() const { return m_Ascending; }
	void ResetSortOrder();
	virtual bool SortColumn(int columnIndex, bool ascending) 
	{ //排序暂时不实现
		return true;
	};


	CSkinHeaderCtrl m_skinHeaderCtrl;
	WNDPROC 	m_oldHeadListProc;
	HWND m_hHeaderWnd;
};