#include "StdAfx.h"
#include "SShowGifWnd.h"
#include <GdiPlus.h>
using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")
#define EVENT_TIEM_ID	100

static ULONG_PTR s_gdipToken=0;

BOOL Gdiplus_Startup()
{
	GdiplusStartupInput gdiplusStartupInput;
	Status st=GdiplusStartup(&s_gdipToken, &gdiplusStartupInput, NULL);
	return st==0;

}

void Gdiplus_Shutdown()
{
	GdiplusShutdown(s_gdipToken);
}

SShowGifWnd::SShowGifWnd(void)
{
	//Gdiplus_Startup();
	m_nFramePosition = 0;
	m_pGifImage = NULL;
	m_pPropertyItem = 0;
}


SShowGifWnd::~SShowGifWnd(void)
{
	DeleteGif();
	//Gdiplus_Shutdown();
}

void SShowGifWnd::OnPaint(IRenderTarget *pRT)
{
	if ( NULL == m_pGifImage ) return;

	GUID pageGuid = Gdiplus::FrameDimensionTime;
	Gdiplus::Graphics graphics( pRT->GetDC() );

	SOUI::CRect rect = GetWindowRect();
	SOUI::CRect m_rcItem = rect;

	if(m_pGifImage->GetWidth()<=rect.Width() && m_pGifImage->GetHeight() <= rect.Width()) //小图片，不缩放  
	{
		SOUI::CPoint x = rect.CenterPoint();
		x.Offset(-(m_pGifImage->GetWidth() / 2) , - (m_pGifImage->GetHeight() / 2));

		SOUI::CPoint y = rect.CenterPoint();
		y.Offset(m_pGifImage->GetWidth() / 2 , m_pGifImage->GetHeight() / 2);

		m_rcItem = SOUI::CRect(x,y);
	}
	else
	{
		//缩放比例
		float xScale=(float)rect.Width()/(float)m_pGifImage->GetWidth();  
		float yScale=(float)rect.Height()/(float)m_pGifImage->GetHeight();  

		float ScaleIndex = (xScale >= yScale ? yScale : xScale);  //选择一个较小的比例

		int width = ScaleIndex * m_pGifImage->GetWidth();
		int height = ScaleIndex * m_pGifImage->GetHeight();


		SOUI::CPoint x = rect.CenterPoint();
		x.Offset(-(width / 2) , - (height / 2));

		SOUI::CPoint y = rect.CenterPoint();
		y.Offset(width / 2 , height / 2);
		m_rcItem = SOUI::CRect(x,y);
	}


	graphics.DrawImage( m_pGifImage, m_rcItem.left, m_rcItem.top, m_rcItem.right-m_rcItem.left, m_rcItem.bottom-m_rcItem.top );
	m_pGifImage->SelectActiveFrame( &pageGuid, m_nFramePosition );
}

void SShowGifWnd::OnTimer(char idEvent)
{
	if ( idEvent != EVENT_TIEM_ID )
		return;

	if ( m_pPropertyItem == NULL )
	{
		return ;
	}

	KillTimer( EVENT_TIEM_ID );
	this->Invalidate();

	m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;

	long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
	if ( lPause == 0 ) lPause = 100;
	SetTimer(EVENT_TIEM_ID, lPause );
}

void SShowGifWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if ( !bShow)
	{
		DeleteGif();
	}

	SetMsgHandled(FALSE);
}

BOOL SShowGifWnd::SetImageFile(LPCTSTR lpszImageFile)
{
	if ( !PathFileExists( lpszImageFile) )
		return FALSE;

	DeleteGif();
	m_nFramePosition  = 0;

	//读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
	HANDLE hFile = ::CreateFile(lpszImageFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) 
		return FALSE;

	DWORD dwSize = ::GetFileSize(hFile, NULL);
	if( dwSize == 0 ) 
		return FALSE;

	DWORD dwRead = 0;
	LPBYTE pData = new BYTE[ dwSize ];
	if ( !pData )
	{
		return FALSE;
	}
	::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
	::CloseHandle( hFile );

	if( dwRead != dwSize ) 
	{
		delete[] pData;
		pData = NULL;
		return FALSE;
	}

	if ( ! LoadGifFromMemory(pData, dwSize) )
	{
		delete pData;
		return FALSE;
	}
	
	delete pData;
	return PlayGif();
}


BOOL SShowGifWnd::LoadGifFromMemory( LPVOID pBuf,size_t dwSize )
{
	HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
	BYTE* pMem = (BYTE*)::GlobalLock(hMem);

	memcpy(pMem, pBuf, dwSize);

	IStream* pStm = NULL;
	::CreateStreamOnHGlobal(hMem, TRUE, &pStm);
	m_pGifImage = Gdiplus::Image::FromStream(pStm);
	if(!m_pGifImage || m_pGifImage->GetLastStatus() != Gdiplus::Ok)
	{
		pStm->Release();
		::GlobalUnlock(hMem);
		return 0;
	}

	UINT nCount	= 0;
	nCount	=	m_pGifImage->GetFrameDimensionsCount();
	GUID* pDimensionIDs	=	new GUID[ nCount ];
	m_pGifImage->GetFrameDimensionsList( pDimensionIDs, nCount );
	m_nFrameCount	=	m_pGifImage->GetFrameCount( &pDimensionIDs[0] );
	int nSize		=	m_pGifImage->GetPropertyItemSize( PropertyTagFrameDelay );
	m_pPropertyItem	=	(Gdiplus::PropertyItem*) malloc( nSize );

	Status st = m_pGifImage->GetPropertyItem( PropertyTagFrameDelay, nSize, m_pPropertyItem );
	if ( st == PropertyNotFound )
	{
		free(m_pPropertyItem);
		m_pPropertyItem = NULL;
	}
	delete  pDimensionIDs;
	pDimensionIDs = NULL;

	return TRUE;
}

BOOL SShowGifWnd::PlayGif()
{
	if ( !m_pPropertyItem )
	{
		return FALSE;
	}

	long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
	if ( lPause == 0 ) lPause = 100;
	return SetTimer(EVENT_TIEM_ID, lPause );
}

void SShowGifWnd::DeleteGif()
{
	return;
	KillTimer(EVENT_TIEM_ID);
	m_nFramePosition = 0;
	if ( m_pGifImage != NULL )
	{
		delete m_pGifImage;
		m_pGifImage = NULL;
	}

	if ( m_pPropertyItem != NULL )
	{
		free( m_pPropertyItem );
		m_pPropertyItem = NULL;
	}
	m_nFrameCount		=	0;	
	m_nFramePosition	=	0;	
}