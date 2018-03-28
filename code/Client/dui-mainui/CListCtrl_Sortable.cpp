#include "stdafx.h"

#include "CListCtrl_Sortable.h"
#include "Resource.h"

#include <shlwapi.h>

// BEGIN_MESSAGE_MAP(CListCtrl_Sortable, CListCtrl)
// 	ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnHeaderClick)	// Column Click
// END_MESSAGE_MAP()

namespace {
	bool IsThemeEnabled()
	{
		HMODULE hinstDll;
		bool XPStyle = false;
		bool (__stdcall *pIsAppThemed)();
		bool (__stdcall *pIsThemeActive)();

		// Test if operating system has themes enabled
		hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pIsAppThemed = ::GetProcAddress(hinstDll, "IsAppThemed");
			(FARPROC&)pIsThemeActive = ::GetProcAddress(hinstDll,"IsThemeActive");
			::FreeLibrary(hinstDll);
			if (pIsAppThemed != NULL && pIsThemeActive != NULL)
			{
				if (pIsAppThemed() && pIsThemeActive())
				{
					// Test if application has themes enabled by loading the proper DLL
					hinstDll = LoadLibrary(_T("comctl32.dll"));
					if (hinstDll)
					{
						DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");
						::FreeLibrary(hinstDll);
						if (pDllGetVersion != NULL)
						{
							DLLVERSIONINFO dvi;
							ZeroMemory(&dvi, sizeof(dvi));
							dvi.cbSize = sizeof(dvi);
							HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
							if (SUCCEEDED(hRes))
                                XPStyle = dvi.dwMajorVersion >= 6;
						}
					}
				}
			}
		}
		return XPStyle;
	}

	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR app, LPCWSTR idlist)
	{
		HMODULE hinstDll;
		HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		HRESULT hRet = S_FALSE;
		hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, "OpenThemeData");
			(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, "CloseThemeData");
			(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, "SetWindowTheme");
			
			if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
			{
				HANDLE theme = pOpenThemeData(hwnd,L"ListView");
				if (theme!=NULL)
				{
					/*_ASSERT(*/pCloseThemeData(theme)==S_OK/*)*/;
					hRet = pSetWindowTheme(hwnd, app, idlist);
				}
			}
			::FreeLibrary(hinstDll);
		}

		return hRet;
	}
}


LRESULT CALLBACK CListCtrl_Sortable::ListViewHeaderWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	CListCtrl_Sortable* pThis = (CListCtrl_Sortable*)::GetWindowLong(hwnd, GWLP_USERDATA);
	

	PAINTSTRUCT ps ={ 0 };
	RECT rect = { 0 };
	HDC hPen = NULL;
	HDC hBrush = NULL;
	int iCount = 0;
	int i1 = 0;
	BYTE red0 = 115, green0 = 154, blue0 = 206;
	BYTE red1 = 255, green1 = 255, blue1 = 255;
	BYTE red, green, blue;
	int j, m, n;
	

	switch(uMsg)
	{
	case WM_PAINT:
		{
			OutputDebugString(_T("aaaaaaaaa"));
		}
		
		break;
	default:
		return CallWindowProc(pThis->m_oldHeadListProc, hwnd,uMsg, wParam, lParam);
	}

	return 0;
}

CListCtrl_Sortable::CListCtrl_Sortable()
	:m_Ascending(false)
	,m_SortCol(-1)

{
	m_oldHeadListProc = NULL;
	m_hHeaderWnd = NULL;
	EnableWindowTheme(this->m_hWnd, L"Explorer", NULL);
}

BOOL CListCtrl_Sortable::OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pLV = reinterpret_cast<NMLISTVIEW*>(pNMHDR);

	SetFocus();	// Ensure other controls gets kill-focus

	int colIndex = pLV->iSubItem;

	if (m_SortCol==colIndex)
	{
		m_Ascending = !m_Ascending;
	}
	else
	{
		m_SortCol = colIndex;
		m_Ascending = true;
	}

	if (SortColumn(m_SortCol, m_Ascending))
		SetSortArrow(m_SortCol, m_Ascending);

	return FALSE;	// Let parent-dialog get chance
}

void CListCtrl_Sortable::SetSortArrow(int colIndex, bool ascending)
{
	if (IsThemeEnabled())
	{
		
#if (_WIN32_WINNT >= 0x501)
		for(int i = 0; i < /*this->GetHeader()->GetItemCount()*/this->GetHeader().GetItemCount(); ++i)
		{
			HDITEM hditem = {0};
			hditem.mask = HDI_FORMAT;
			/*VERIFY( GetHeaderCtrl()->GetItem( i, &hditem ) );*/
			/*_ASSERT(*/GetHeader().GetItem(i, &hditem)/*)*/;
			hditem.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
			if (i == colIndex)
			{
				hditem.fmt |= ascending ? HDF_SORTDOWN : HDF_SORTUP;
			}
			/*VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );*/
			/*_ASSERT(*/GetHeader().SetItem(i, &hditem)/*)*/;
		}
#endif
	}
	else
	{
		UINT bitmapID = m_Ascending ? IDB_DOWNARROW : IDB_UPARROW; 
		for(int i = 0; i < /*GetHeaderCtrl()->GetItemCount()*/GetHeader().GetItemCount(); ++i)
		{
			HDITEM hditem = {0};
			hditem.mask = HDI_BITMAP | HDI_FORMAT;
			//VERIFY( GetHeaderCtrl()->GetItem( i, &hditem ) );
			/*_ASSERT(*/GetHeader().GetItem(i, &hditem)/*)*/;
			if (hditem.fmt & HDF_BITMAP && hditem.fmt & HDF_BITMAP_ON_RIGHT)
			{
				if (hditem.hbm)
				{
					DeleteObject(hditem.hbm);
					hditem.hbm = NULL;
				}
				hditem.fmt &= ~(HDF_BITMAP|HDF_BITMAP_ON_RIGHT);
				//VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );
				/*_ASSERT(*/GetHeader().SetItem(i, &hditem)/*)*/;
			}
			if (i == colIndex)
			{
				hditem.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
				hditem.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS); 
				//VERIFY( hditem.hbm!=NULL );
				//_AERTSS(hditem.hbm != NULL);
				//VERIFY( CListCtrl::GetHeaderCtrl()->SetItem( i, &hditem ) );
				/*_ASSERT(*/GetHeader().SetItem(i, &hditem)/*)*/;
			}
		}
	}
}

void CListCtrl_Sortable::PreSubclassWindow()
{
	//CListCtrl::PreSubclassWindow();

	//CListViewCtrl:();
	// Focus retangle is not painted properly without double-buffering
#if (_WIN32_WINNT >= 0x501)
	//this->SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
	//CListViewCtrl::SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | GetExtenl());
#endif
//	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	//SetExtendedStyle(GetExtendedStyle() | LVS_EX_HEADERDRAGDROP);

	//EnableWindowTheme(GetSafeHwnd(), L"Explorer", NULL);
	EnableWindowTheme(this->m_hWnd, L"Explorer", NULL);
}

void CListCtrl_Sortable::ResetSortOrder()
{
	m_Ascending = true;
	m_SortCol = -1;
	SetSortArrow(m_SortCol, m_Ascending);
}

// The column version of GetItemData(), one can specify an unique
// identifier when using InsertColumn()
int CListCtrl_Sortable::GetColumnData(int col) const
{
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_SUBITEM;
	//VERIFY( GetColumn(col, &lvc) );
	/*_ASSERT(*/GetColumn(col, &lvc)/*)*/;
	return lvc.iSubItem;
}

LRESULT CListCtrl_Sortable::OnCreate(LPCREATESTRUCT lParam)
{
	m_hHeaderWnd = FindWindowEx(m_hWnd, NULL, _T("SysHeader32"),NULL);
	m_skinHeaderCtrl.SubclassWindow(m_hHeaderWnd);
	//SetBkColor()
	/*
	m_hHeaderWnd = FindWindowEx(m_hWnd, NULL, _T("SysHeader32"),NULL);
	m_oldHeadListProc =  (WNDPROC)::GetWindowLong(m_hHeaderWnd, GWL_WNDPROC);

	::SetWindowLong(m_hHeaderWnd, GWLP_USERDATA, (long)this);
	::SetWindowLong(m_hHeaderWnd, GWL_WNDPROC, (long)ListViewHeaderWindowProc);
	*/
	return 0;
}

LRESULT CListCtrl_Sortable::OnDrawItem(WPARAM wParam, LPDRAWITEMSTRUCT drawItem)
{
	return 0;
}

CListCtrl_Sortable::~CListCtrl_Sortable()
{
	if (m_hHeaderWnd && ::IsWindow(m_hHeaderWnd))
	{
		::DestroyWindow(m_hHeaderWnd);
	}

	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		::DestroyWindow(m_hWnd);
	}
}
