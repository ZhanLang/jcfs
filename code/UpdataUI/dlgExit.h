#pragma once


class CDlgExit : public WindowImplBase
{
public:
	CDlgExit();
	~CDlgExit();

	bool NeedExit(){return m_bExit;}

protected:
	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

protected:
	virtual CDuiString GetSkinFolder() {return _T("updata_skin");}
	virtual CDuiString GetSkinFile() {return _T("dlgExit.xml");}
	virtual LPCTSTR GetWindowClassName(void) const {return _T("UI_UPDATA");}

protected:
	bool m_bExit;
};
