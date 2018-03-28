#if !defined(AFX_SKINHEADERCTRL_H__8B0847B1_B4E6_4372_A62D_038582FFEA5C__INCLUDED_)
#define AFX_SKINHEADERCTRL_H__8B0847B1_B4E6_4372_A62D_038582FFEA5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinHeaderCtrl.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CSkinHeaderCtrl window

class CSkinHeaderCtrl : //public CHeaderCtrlT<CWindowImpl<CHeaderCtrlT>>
	//public CHeaderCtrl,
	public CWindowImpl<CHeaderCtrl>
{
// Construction
public:
	CSkinHeaderCtrl();

// Attributes
public:
	BEGIN_MSG_MAP_EX(CSkinHeaderCtrl)

		//NOTIFY_HANDLER(,OnNotify)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MESSAGE_HANDLER(HDM_LAYOUT,OnLayout)
	
		END_MSG_MAP()
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkinHeaderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual ~CSkinHeaderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSkinHeaderCtrl)
	void OnPaint(HDC hdc);
	BOOL OnEraseBkgnd(HDC pDC);
	LRESULT OnLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHander );
	//}}AFX_MSG

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINHEADERCTRL_H__8B0847B1_B4E6_4372_A62D_038582FFEA5C__INCLUDED_)
