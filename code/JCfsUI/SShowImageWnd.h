#pragma once
#include <imagestone/ImageStone.h>
class SShowImageWnd : public SWindow
{
	 SOUI_CLASS_NAME(SImageWnd, L"image")
public:
	SShowImageWnd(void);
	~SShowImageWnd(void);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_SHOWWINDOW(OnShowWindow)
	SOUI_MSG_MAP_END()

public:
	BOOL SetImageFile( LPCTSTR lpszImageFile);
	void OnShowWindow(BOOL bShow, UINT nStatus);

protected:
	void OnPaint(IRenderTarget *pRT);

private:
	FCObjImage        m_img ;
};

