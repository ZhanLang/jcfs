#include "stdafx.h"


HIMAGELIST CIconViewUI::g_hImageList[2] = {0};
CIconViewUI::CIconViewUI()
{
	m_UseLarge = FALSE;
}

/*virtual*/ CIconViewUI::~CIconViewUI()
{
	DelIcon();
}

VOID CIconViewUI::DelIcon()
{
	if (m_hIcon)
	{
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}
}


LPCTSTR CIconViewUI::GetClass() const
{
	return _T("IconView");
}

LPVOID CIconViewUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, DUI_CTR_ICONVIEW) == 0 ) return static_cast<CIconViewUI*>(this);
	return CControlUI::GetInterface(pstrName);
}

UINT CIconViewUI::GetControlFlags() const
{
	return 0;
}

/*virtual*/ void CIconViewUI::DoInit()
{
	if (!(g_hImageList[0] && g_hImageList[1]))
	{
		SHFILEINFO shfi;
		ZeroMemory(&shfi,sizeof(SHFILEINFO));
		TCHAR dir[MAX_PATH] = {0};
		GetSystemDirectory(dir,MAX_PATH);
		dir[3]=0;

		//»ñÈ¡Í¼±ê
		g_hImageList[SHGFI_SMALLICON] = (HIMAGELIST)SHGetFileInfo(dir,0,&shfi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
		g_hImageList[SHGFI_LARGEICON] = (HIMAGELIST)SHGetFileInfo(dir,0,&shfi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_LARGEICON);
	}


	//ImageList_GetIcon
}

VOID	CIconViewUI::InitIcon(INT nIconIndex)
{
	DelIcon();
	if (m_UseLarge)
	{
		if (g_hImageList[SHGFI_LARGEICON])
		{
			m_hIcon = ImageList_GetIcon(g_hImageList[SHGFI_LARGEICON], nIconIndex, ILD_TRANSPARENT);
		}	
	}

	else if (g_hImageList[SHGFI_SMALLICON])
	{
		m_hIcon = ImageList_GetIcon(g_hImageList[SHGFI_LARGEICON], nIconIndex, ILD_TRANSPARENT);
	}
}

VOID   CIconViewUI::InitIcon(LPCTSTR lpszFile)
{
	DelIcon();
	m_hIcon = (HICON)LoadImage(NULL, lpszFile, IMAGE_ICON, this->GetWidth(), this->GetHeight(), LR_LOADFROMFILE );
}

/*virtual*/ void CIconViewUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	DoInit();
	if (_tcsicmp(pstrName, _T("file")) == 0)
	{
		m_strFile = pstrValue;
		InitIcon(m_strFile);
	}
	else if (_tcsicmp(pstrName, _T("index")) == 0)
	{
		m_nIconIndex = _ttoi(pstrValue);
		InitIcon(m_nIconIndex);
	}
	else
		CControlUI::SetAttribute(pstrName, pstrValue);
}

/*virtual*/ void CIconViewUI::PaintStatusImage(HDC hDC)
{
	CDuiRect rect = m_rcItem;
	DrawIconEx(hDC, rect.left,rect.top,m_hIcon,rect.GetWidth(),rect.GetHeight(),0,NULL,DI_NORMAL);
}