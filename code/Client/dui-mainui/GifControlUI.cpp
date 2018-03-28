#include "StdAfx.h"
#include "GifControlUI.h"


CGifControlUI::CGifControlUI(void)
{
	m_nFramePos = 0;
}


CGifControlUI::~CGifControlUI(void)
{
}


LPCTSTR CGifControlUI::GetClass() const
{
	return _T("WTLVirtualListoUI");
}

LPVOID CGifControlUI::GetInterface(LPCTSTR pstrName)
{
	return CControlUI::GetInterface(pstrName);
}

void CGifControlUI::DoInit()
{
	//m_pManager->SetTimer(this, 2008, 100)
}

UINT CGifControlUI::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

void CGifControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("gifres")) == 0)
	{
		m_gifImage.LoadFromResource( m_pManager->GetResourceDll(), MAKEINTRESOURCE(_ttoi(pstrValue)) , _T("GIF"));
	}
	else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

void CGifControlUI::PaintBkImage(HDC hDC)
{
	if (IsVisible())
	{
		m_gifImage.SelectActiveFrame(m_nFramePos);
		m_gifImage.Draw(hDC, GetPos(),m_gifImage.GetCurFramePos());
	}
	
	//CControlUI::PaintBkImage(hDC);
}

void CGifControlUI::Event(TEventUI& event)
{
	if (event.wParam == 2008 && IsVisible())
	{
		m_pManager->KillTimer(this,2008);

		m_nFramePos++;
		if (m_nFramePos >= (int)m_gifImage.GetFrameCount())
			m_nFramePos = 0;
		
		if (m_gifImage.GetFrameDelay(m_nFramePos))
		{
			m_pManager->SetTimer(this, 2008,  m_gifImage.GetFrameDelay(m_nFramePos));
		}
		
		this->Invalidate();
	}
}

void CGifControlUI::SetVisible(bool bVisible/* = true*/)
{
	if (bVisible)
	{
		if (m_pManager)
		{
			//this->SetMaxWidth(m_gifImage.GetWidth());
			//this->SetMaxHeight(m_gifImage.GetHeight());

			int dwParentHeight = this->GetParent()->GetHeight();
			int dwParentWidth = this->GetParent()->GetWidth();

			if (!dwParentWidth)
			{
				dwParentWidth = this->GetParent()->GetFixedWidth();
			}
			if (!dwParentHeight)
			{
				dwParentHeight = this->GetParent()->GetFixedHeight();
			}

			RECT rect = {0};
			if (dwParentHeight - m_gifImage.GetHeight() > 0)
			{
				this->SetMaxHeight(m_gifImage.GetHeight());
				rect.top = (dwParentHeight - this->GetMaxHeight()) / 2;
			}
			else
			{
				this->SetMaxHeight(dwParentHeight);
			}


			if (dwParentWidth - m_gifImage.GetHeight() > 0)
			{
				this->SetMaxWidth(m_gifImage.GetWidth());
				rect.left = (dwParentWidth - this->GetMaxWidth()) /2;
			}
			else
			{
				this->SetMaxWidth(dwParentWidth);
			}

			this->SetPadding(rect);

			m_gifImage.SelectActiveFrame(0);
			if (m_gifImage.GetFrameDelay(-1))
			{
				m_pManager->SetTimer(this, 2008, m_gifImage.GetFrameDelay(-1));
			}
		}
		
	}
	else
	{
		if (m_pManager)
		{
			m_pManager->KillTimer(this,2008);
		}
		
	}

	CControlUI::SetVisible(bVisible);
}

void CGifControlUI::SetPathFile(LPCTSTR lpszFile)
{
	m_gifImage.Destroy();
	m_gifImage.LoadFromFile(lpszFile);
	Invalidate();
}


// int CGifControlUI::GetWidth() const
// {
// 	return m_gifImage.GetWidth();
// }
// int CGifControlUI::GetHeight() const
// {
// 	return m_gifImage.GetHeight();
// }
