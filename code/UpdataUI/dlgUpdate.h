#pragma once


class CDlgUpdate : public WindowImplBase
{
public:
	CDlgUpdate();
	~CDlgUpdate();

	static bool NeedTip();
	static void SetTip(bool bTip);

	static bool AutoUpdate();
	static void SetAutoUpdate(bool bAuto);

protected:
DUI_DECLARE_MESSAGE_MAP()
	virtual void OnWinInit(TNotifyUI& msg);
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelChange(TNotifyUI& msg);

protected:
	virtual CDuiString GetSkinFolder() {return _T("res_instIMM");}
	virtual CDuiString GetSkinFile() {return _T("dlgUpdate.xml");}
	virtual LPCTSTR GetWindowClassName(void) const {return _T("UI_XmlLGOSInsterDlgUpdate");}

protected:
	COptionUI* m_opt;
};
