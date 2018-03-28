#pragma once

namespace Gdiplus
{
	class Bitmap;
	class Image;
	class PropertyItem;
}

class SShowGifWnd : public SWindow
{
	SOUI_CLASS_NAME(SImageWnd, L"sgif")
public:
	SShowGifWnd(void);
	~SShowGifWnd(void);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SHOWWINDOW(OnShowWindow)
	SOUI_MSG_MAP_END()

protected:
	void OnPaint(IRenderTarget *pRT);
	void OnTimer( char idEvent );
	void OnShowWindow(BOOL bShow, UINT nStatus);
public:
	BOOL SetImageFile( LPCTSTR lpszImageFile);

private:
	BOOL LoadGifFromMemory( LPVOID pBuf,size_t dwSize );
	BOOL PlayGif();
	VOID DeleteGif();
private:
	Gdiplus::Image	*m_pGifImage;
	UINT			m_nFrameCount;				// gif图片总帧数
	UINT			m_nFramePosition;			// 当前放到第几帧
	Gdiplus::PropertyItem*	m_pPropertyItem;	// 帧与帧之间间隔时间
};

