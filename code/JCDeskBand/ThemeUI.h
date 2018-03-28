#pragma once
class CThemeUI : 
	public CVerticalLayoutUI,
	public IMessageFilterUI
{
public:
	DECLARE_DUICONTROL(CThemeUI)

	LPCTSTR GetClass() const
	{
		return _T("Theme" );
	}


	CThemeUI(void);
	~CThemeUI(void);
	virtual void Init();
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

	virtual void DoPaint(HDC hDC, const RECT& rcPaint);
	HTHEME m_hTheme;
};

