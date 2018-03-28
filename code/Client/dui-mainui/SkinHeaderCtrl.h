
#pragma once
#include "atlctrls.h"

class CSkinHeaderCtrl:
	public CWindowImpl<CHeaderCtrl>
{
public:
	CSkinHeaderCtrl();

public:
	BEGIN_MSG_MAP_EX(CSkinHeaderCtrl)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MESSAGE_HANDLER(HDM_LAYOUT,OnLayout)
	END_MSG_MAP()


public:
	//virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual ~CSkinHeaderCtrl();

	// Generated message map functions
protected:
	void OnPaint(HDC hdc);
	BOOL OnEraseBkgnd(HDC pDC);
	LRESULT OnLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHander );

};
