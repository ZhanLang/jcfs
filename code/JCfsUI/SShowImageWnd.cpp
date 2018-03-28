#include "StdAfx.h"
#include "SShowImageWnd.h"


SShowImageWnd::SShowImageWnd(void)
{
}


SShowImageWnd::~SShowImageWnd(void)
{
}

BOOL SShowImageWnd::SetImageFile(LPCTSTR lpszImageFile)
{

	if ( !PathFileExists(lpszImageFile) )
		return FALSE;

	if ( m_img.IsValidImage() )
		m_img.Destroy();

	FCImageProperty   prop ;
	if ( !m_img.Load (lpszImageFile, &prop) )
		return FALSE;
	
	if (m_img.IsValidImage() && (m_img.ColorBits() == 32))
	{
		m_img.ApplyEffect (FCEffectPremultipleAlpha()) ;
	}
	return TRUE;
}

void SShowImageWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if ( !bShow )
	{
		//if ( m_img.IsValidImage() )
			//m_img.Destroy();
	}

	SetMsgHandled(FALSE);
}

void SShowImageWnd::OnPaint(IRenderTarget *pRT)
{
	SOUI::CRect rect = GetWindowRect();
	if ( !m_img.IsValidImage() )
		return ;

	if(m_img.Width()<=rect.Width() && m_img.Height() <= rect.Width()) //小图片，不缩放  
	{
		SOUI::CPoint x = rect.CenterPoint();
		x.Offset(-(m_img.Width() / 2) , - (m_img.Height() / 2));

		SOUI::CPoint y = rect.CenterPoint();
		y.Offset(m_img.Width() / 2 , m_img.Height() / 2);

		m_img.Draw(pRT->GetDC(),SOUI::CRect(x,y));
	}
	else
	{
		//缩放比例
		float xScale=(float)rect.Width()/(float)m_img.Width();  
		float yScale=(float)rect.Height()/(float)m_img.Height();  

		float ScaleIndex = (xScale >= yScale ? yScale : xScale);  //选择一个较小的比例

		int width = ScaleIndex * m_img.Width();
		int height = ScaleIndex * m_img.Height();


		SOUI::CPoint x = rect.CenterPoint();
		x.Offset(-(width / 2) , - (height / 2));

		SOUI::CPoint y = rect.CenterPoint();
		y.Offset(width / 2 , height / 2);

		m_img.Draw(pRT->GetDC(),SOUI::CRect(x,y));
	}
}
