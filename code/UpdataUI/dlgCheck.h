#pragma once

class CDlgCheckUpdate : public WindowImplBase
{
	enum {PAGE_CHECK, PAGE_NEED, PAGE_ERR, PAGE_NOTNEED};
public:
	CDlgCheckUpdate();
	~CDlgCheckUpdate();

protected:
DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnWinInit(TNotifyUI& msg);

protected:
	 virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual CDuiString GetSkinFolder() {return _T("res_instIMM");}
	virtual CDuiString GetSkinFile() {return _T("dlgCheckUpdate.xml");}
	virtual LPCTSTR GetWindowClassName(void) const {return _T("UI_XmlLGOSInsterDlgCheckUp");}

	virtual CControlUI* CreateControl(LPCTSTR pstrClass);

protected:
	CTabLayoutUI* m_tab;
	CUpdate m_update;
};
