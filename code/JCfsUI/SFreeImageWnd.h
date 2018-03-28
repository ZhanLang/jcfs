#pragma once
#include <freeimage.h>
namespace Gdiplus
{
	class Bitmap;
	class Image;
}
class SFreeImageWnd : public SWindow
{
	 SOUI_CLASS_NAME(SImageWnd, L"freeimage")
public:

	static bool mem_to_global( const void *buf, size_t size, HGLOBAL global );

	static Gdiplus::Image *mi_from_memory( const void *buf, size_t size );
public:


	SFreeImageWnd(void);
	~SFreeImageWnd(void);
	BOOL SetImage(LPCTSTR lpszFile);

	  SOUI_MSG_MAP_BEGIN()
		  MSG_WM_PAINT_EX(OnPaint)
		  MSG_WM_SHOWWINDOW(OnShowWindow)
	 SOUI_MSG_MAP_END() 
protected:
	 void OnPaint(IRenderTarget *pRT);
	 void OnShowWindow(BOOL bShow, UINT nStatus);
private:
	FREE_IMAGE_FORMAT m_fif;
	FIBITMAP*		m_dib;
	CString m_strFile;
	Gdiplus::Image*		m_pImage;
	Gdiplus::Bitmap* pBitmap;
	int m_nImageWidth;
	int m_nImageHeight;
};

