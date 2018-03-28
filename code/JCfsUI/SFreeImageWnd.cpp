#include "StdAfx.h"
#include "SFreeImageWnd.h"

#include <GdiPlus.h>
using namespace Gdiplus;

#pragma comment(lib,"gdiplus.lib")
bool SFreeImageWnd::mem_to_global( const void *buf, size_t size, HGLOBAL global )
	{
		void *dest = ::GlobalLock( global );
		if( dest == NULL )
		{
			return false;
		}
		memcpy( dest, buf, size );
		::GlobalUnlock( global );
		return true;
	}

	Gdiplus::Image *SFreeImageWnd::mi_from_memory( const void *buf, size_t size )
	{
		IStream *stream = NULL;
		HGLOBAL global = ::GlobalAlloc( GMEM_FIXED, size );
		if( global == NULL )
		{
			return NULL;
		}
		/* copy the buf content to the HGLOBAL */
		if( !mem_to_global( buf, size, global ) )
		{
			::GlobalFree( global );
			return NULL;
		}
		/* get the IStream from the global object */
		if( ::CreateStreamOnHGlobal( global, TRUE, &stream ) != S_OK )
		{
			::GlobalFree( global );
			return NULL;
		}
		/* create the image from the stream */
		Gdiplus::Image *image = Gdiplus::Image::FromStream( stream );

		stream->Release();
		/* i suppose when the reference count for stream is 0, it will 
		GlobalFree automatically. The Image maintain the object also.*/	
		return image;
	}

SFreeImageWnd::SFreeImageWnd(void)
{
	m_fif = FIF_UNKNOWN;
	m_dib = NULL;
}


SFreeImageWnd::~SFreeImageWnd(void)
{
}

BOOL SFreeImageWnd::SetImage(LPCTSTR lpszFile)
{
	if ( m_dib )
	{
		FreeImage_Unload( m_dib );
		m_dib = NULL;
	}

	m_strFile = lpszFile;
	m_fif = FreeImage_GetFileType(CT2A(m_strFile));
	if ( m_fif == FIF_UNKNOWN)
		return FALSE;

	if( !FreeImage_FIFSupportsReading(m_fif) )
		return FALSE;

	m_dib = FreeImage_Load(m_fif,CT2A(m_strFile),0); 
	if ( !m_dib )
		return FALSE;
	

	int x = FreeImage_GetWidth(m_dib);    
	int y = FreeImage_GetHeight(m_dib);

	BYTE* pImage = FreeImage_GetBits(m_dib); 
	BITMAPINFO* pBitmapInfo = FreeImage_GetInfo(m_dib);
	
	return TRUE;
}

void SFreeImageWnd::OnPaint(IRenderTarget *pRT)
{
	//需要计算一下宽高比例
	SOUI::CRect rect = GetWindowRect();
	SOUI::CRect m_rcItem = rect;

	if(FreeImage_GetWidth(m_dib)<=rect.Width() && FreeImage_GetHeight(m_dib) <= rect.Width()) //小图片，不缩放  
	{
		SOUI::CPoint x = rect.CenterPoint();
		x.Offset(-(FreeImage_GetWidth(m_dib) / 2) , - (FreeImage_GetHeight(m_dib) / 2));

		SOUI::CPoint y = rect.CenterPoint();
		y.Offset(FreeImage_GetWidth(m_dib) / 2 , FreeImage_GetHeight(m_dib) / 2);

		m_rcItem = SOUI::CRect(x,y);
	}
	else
	{
		//缩放比例
		float xScale=(float)rect.Width()/(float)FreeImage_GetWidth(m_dib);  
		float yScale=(float)rect.Height()/(float)FreeImage_GetHeight(m_dib);  

		float ScaleIndex = (xScale >= yScale ? yScale : xScale);  //选择一个较小的比例

		int width = ScaleIndex * FreeImage_GetWidth(m_dib);
		int height = ScaleIndex * FreeImage_GetHeight(m_dib);


		SOUI::CPoint x = rect.CenterPoint();
		x.Offset(-(width / 2) , - (height / 2));

		SOUI::CPoint y = rect.CenterPoint();
		y.Offset(width / 2 , height / 2);
		m_rcItem = SOUI::CRect(x,y);
	}

	BYTE* pImgData = FreeImage_GetBits(m_dib);  
	BITMAPINFO* pBitmapInfo = FreeImage_GetInfo(m_dib);
	::SetStretchBltMode(pRT->GetDC(),STRETCH_HALFTONE);
	::StretchDIBits(pRT->GetDC(),m_rcItem.left,m_rcItem.top,m_rcItem.Width(),m_rcItem.Height(),0,0,FreeImage_GetWidth(m_dib),FreeImage_GetHeight(m_dib),    
		(const void*)pImgData,pBitmapInfo,DIB_RGB_COLORS,SRCCOPY);    
}

void SFreeImageWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if ( !bShow)
	{
		FreeImage_Unload( m_dib );
		m_dib = NULL;
	}

	SetMsgHandled(FALSE);
}
