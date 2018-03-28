// SkinHeaderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "SkinHeaderCtrl.h"



/////////////////////////////////////////////////////////////////////////////
// CSkinHeaderCtrl

CSkinHeaderCtrl::CSkinHeaderCtrl()
{
}

CSkinHeaderCtrl::~CSkinHeaderCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSkinHeaderCtrl message handlers

void CSkinHeaderCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	
}

LRESULT CSkinHeaderCtrl::OnLayout(UINT uMsg, WPARAM wParam, LPARAM lParam , BOOL & bHander)
{
	//LRESULT lResult = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, 0, lParam);
	
	LRESULT  lResult;
	lResult = __super::DefWindowProc(uMsg,wParam, lParam);

	bHander = TRUE;
	HD_LAYOUT &hdl = *( HD_LAYOUT * ) lParam;
	
	RECT *prc = hdl.prc;
	
	WINDOWPOS *pwpos = hdl.pwpos;
	
	int nHeight = (int)(pwpos->cy * 2); //改变高度,m_Height为倍数
	
	pwpos->cy = 24;
	prc->top = 24;
	//prc->bottom += 4;
	//prc->right += 4;
	//prc->top += 4;
	//prc->left +=4;
	//prc->top = 0;
	//pwpos
	return 0;

	return S_OK;
}

void CSkinHeaderCtrl::OnPaint(HDC _hdc) 
{
	CPaintDC dc(this->m_hWnd); // device context for painting
	
	
	SOUI::CRect rect, rectItem, clientRect;
	GetClientRect(&rect);
	GetClientRect(&clientRect);
	
	CMemoryDC memDC(dc.m_hDC, rect);
	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(dc.m_hDC);
	
	memDC.FillSolidRect(&rect, RGB(76,85,118));

	CBitmap bitmapSpan;
	bitmapSpan.LoadBitmap(IDB_COLUMNHEADER_SPAN);
	HBITMAP pOldBitmapSpan = bitmapDC.SelectBitmap(bitmapSpan.m_hBitmap);

	memDC.StretchBlt(rect.left+2, 0, rect.Width(), 24, bitmapDC.m_hDC, 0, 0, 1, 24, SRCCOPY);

	bitmapDC.SelectBitmap(pOldBitmapSpan);
	bitmapSpan.DeleteObject();
	
	int nItems = /*GetItemCount()*/Header_GetItemCount(m_hWnd);

	CBitmap bitmap;
	CBitmap bitmap2;
	CBitmap bitmap3;
	
	bitmap.LoadBitmap(IDB_COLUMNHEADER_START);
	bitmap2.LoadBitmap(IDB_COLUMNHEADER_SPAN);
	bitmap3.LoadBitmap(IDB_COLUMNHEADER_END);

	for(int i = 0; i <nItems; i++)
	{
		
		TCHAR buf1[256] = { 0 };
		HD_ITEM hditem1;
		
		hditem1.mask = HDI_TEXT | HDI_FORMAT | HDI_ORDER;
		hditem1.pszText = buf1;
		hditem1.cchTextMax = 255;
		//GetItem( i, &hditem1 );
		Header_GetItem(m_hWnd,i, &hditem1);
		//GetItemRect(i, &rect);
		
		Header_GetItemRect(m_hWnd, i, &rect); // 获取Item的高度

		HBITMAP pOldBitmap = NULL;
		
		//make sure we draw the start piece
		//on the first item so it has a left border

		//For the following items we will just use the
		//right border of the previous items as the left
		//border
		if(hditem1.iOrder==0)
		{
			pOldBitmap = bitmapDC.SelectBitmap(bitmap.m_hBitmap);
			memDC.BitBlt(rect.left,rect.top,2,24,bitmapDC.m_hDC,0,0,SRCCOPY);
		}
		else
		{
			memDC.BitBlt(rect.left-1,rect.top,2,24,bitmapDC.m_hDC,0,0,SRCCOPY);
			pOldBitmap = (bitmapDC.SelectBitmap(bitmap2.m_hBitmap));
			memDC.BitBlt(rect.left+1,rect.top,1,24,bitmapDC.m_hDC,0,0,SRCCOPY);
		}

		bitmapDC.SelectBitmap(pOldBitmap);
		
		//span the bitmap for the width of the column header item
		int nWidth = rect.Width() - 4;
		
		HBITMAP pOldBitmap2 = bitmapDC.SelectBitmap(bitmap2.m_hBitmap);
		
		memDC.StretchBlt(rect.left+2, 0, nWidth, 1, bitmapDC.m_hDC, 0,0, 1, 24, SRCCOPY);

		bitmapDC.SelectBitmap(pOldBitmap2);
		
		
		//draw the end piece of the column header
		HBITMAP pOldBitmap3 = bitmapDC.SelectBitmap(bitmap3.m_hBitmap);
		memDC.BitBlt((rect.right-2), 0, 2, 24, bitmapDC.m_hDC,0,0,SRCCOPY);
		bitmapDC.SelectBitmap(pOldBitmap3);
		
		//Get all the info for the current
		//item so we can draw the text to it
		//in the desired font and style
		DRAWITEMSTRUCT	DrawItemStruct = { 0 };
		//GetItemRect(i, &rectItem);
		Header_GetItemRect(m_hWnd, i, &rectItem); // 获取Item的高度
		
		DrawItemStruct.CtlType		= 100;
		DrawItemStruct.hDC			= dc.m_hDC;
		DrawItemStruct.itemAction	= ODA_DRAWENTIRE; 
		DrawItemStruct.hwndItem 	= m_hWnd; 
		DrawItemStruct.rcItem	= rectItem;
		DrawItemStruct.itemID	= i;
		DrawItem(&DrawItemStruct);
		
		UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_TOP |DT_LEFT| DT_END_ELLIPSIS ;
		
		
		CFont font;
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight = 19;
		_tcscpy(lf.lfFaceName, _T("微软雅黑"));
		font.CreateFontIndirect(&lf);
		HFONT def_font = memDC.SelectFont(font);
		
		memDC.SetBkMode(TRANSPARENT);
		rectItem.DeflateRect(2,2,2,2);
		
		TCHAR buf[256] = {0};
		HD_ITEM hditem;
		
		hditem.mask = HDI_TEXT | HDI_FORMAT | HDI_ORDER;
		hditem.pszText = buf;
		hditem.cchTextMax = 255;
	//	GetItem( DrawItemStruct.itemID, &hditem );
		Header_GetItem(m_hWnd,DrawItemStruct.itemID, &hditem);

		SIZE size = {0,0};
		::GetTextExtentPoint32( memDC,buf, _tcslen(buf) , &size);
		memDC.DrawText(buf, -1, &rectItem, uFormat);
		memDC.SelectFont(def_font);
		font.DeleteObject();
	}		
}

BOOL CSkinHeaderCtrl::OnEraseBkgnd(HDC pDC) 
{
	return false;	
}
