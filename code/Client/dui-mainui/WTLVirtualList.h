

#pragma once
#include "CListCtrl_Sortable.h"



class CWTLVirtualList:
	public DuiLib::CControlUI,
	public CListCtrl_Sortable
{
public:
	CWTLVirtualList(void);
	~CWTLVirtualList(void);


	//CSkinVitrualListCtrl
	void DoInit();
	
	BEGIN_MSG_MAP_EX(CWTLVirtualList)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_NOTIFY(OnNotify)
	END_MSG_MAP()


	LRESULT OnCreate(LPCREATESTRUCT lParam);
	virtual LPCTSTR GetClass() const;
	virtual LPVOID GetInterface(LPCTSTR pstrName);
	virtual UINT GetControlFlags() const;

	virtual void SetPos(RECT rc);
	virtual void SetVisible(bool bVisible = true);

	virtual void DoEvent(DuiLib::TEventUI& event);
	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	//////////////////////////////////////////////////////////////////////////
	virtual LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
	virtual void	SetFocus();


	virtual LPCTSTR GetWindowClassName() const;
	virtual LPCTSTR GetSuperClassName() const;


	//virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnNotify(int idCtrl, LPNMHDR lParam);
	
};

