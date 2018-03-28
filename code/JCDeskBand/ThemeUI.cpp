#include "StdAfx.h"
#include "ThemeUI.h"

#include <shlobj.h>

#include <uxtheme.h>

#include <vsstyle.h>
#include <vssym32.h>
#include <gdiplus.h>
using namespace Gdiplus;

IMPLEMENT_DUICONTROL(CThemeUI);
CThemeUI::CThemeUI(void)
{
	m_hTheme = NULL;
}


CThemeUI::~CThemeUI(void)
{
	m_pManager->RemoveMessageFilter(this);
	if ( m_hTheme )
	{
		CloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}
}

void CThemeUI::Init()
{
	m_hTheme = ::OpenThemeData(m_pManager->GetPaintWindow(), L"REBAR");
	m_pManager->AddMessageFilter(this);
}

LRESULT CThemeUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if ( uMsg == WM_THEMECHANGED)
	{
		if ( m_hTheme )
			CloseThemeData(m_hTheme);


		m_hTheme = ::OpenThemeData(m_pManager->GetPaintWindow(), L"REBAR");
	}
	
	return 0;
}

void CThemeUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	CDCHandle dc( hDC );
	CDC dcRebar(::GetDC(::GetParent(m_pManager->GetPaintWindow())));
	CRect rcClient,rcSnap;
	rcClient = GetClientPos();
	rcSnap = rcClient;

	if(IsThemeActive())
	{
		int nAlpha = 0;
		COLORREF cr = {0};

		::GetThemeInt(m_hTheme, RP_BAND, 0, TMT_ALPHATHRESHOLD, &nAlpha);
		::GetThemeInt(m_hTheme, RP_BACKGROUND, 0, TMT_ALPHATHRESHOLD, &nAlpha);
		::GetThemeColor(m_hTheme, RP_BAND, 0, TMT_BLENDCOLOR, &cr);
		SolidBrush sbr(Color(nAlpha, GetRValue(cr), GetRValue(cr), GetRValue(cr))); //Color(/*50*/2, 0, 0, 0)

		Graphics gp(dc);
		gp.FillRectangle(&sbr, 0, 0, rcClient.Width(), rcClient.Height());
		::MapWindowPoints(m_pManager->GetPaintWindow(), ::GetParent(m_pManager->GetPaintWindow()), (LPPOINT)&rcSnap, 2);
		dc.StretchBlt(0, 0, rcClient.Width(), rcClient.Height(), dcRebar, rcSnap.left - 1,  rcSnap.top, 1, rcSnap.Height(), SRCCOPY);
	}

	CVerticalLayoutUI::DoPaint(hDC, rcPaint);
}
