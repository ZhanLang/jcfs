#pragma once

#include "UIlib.h"
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES


class CWTLEdit:
		public DuiLib::CControlUI,
		public CSkinEdit 
{
public:
	CWTLEdit(void);
	~CWTLEdit(void);

	void DoInit();
	//void DoUninit
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
};

