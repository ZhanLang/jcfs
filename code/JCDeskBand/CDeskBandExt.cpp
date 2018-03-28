// CDeskBandExt.cpp : CDeskBandExt µÄÊµÏÖ

#include "stdafx.h"
#include "CDeskBandExt.h"
#include <shlobj.h>

#include <uxtheme.h>

#include <vsstyle.h>
#include <vssym32.h>
#include <gdiplus.h>
//#include "TransEditUI.h"
using namespace Gdiplus;
#define UPDATE_TIMER_ID 0x1520  
#define IDM_COMMAND  0 


#include "DuiCore\IDuiSkin.h"
#include <DuiImpl\DuiString.h>
#include <DuiCore\IDuiLang.h>


// CDeskBandExt
#pragma comment(lib,"Gdiplus.lib")
CDeskBandExt::CDeskBandExt() :   
	m_lastString(TEXT("Perry")),  
	m_sl(0)  
{  
	ATLTRACE("CDeskBandExt::CDeskBandExt");  
	m_hMenu = ::CreatePopupMenu();  

	::AppendMenu(m_hMenu, MF_STRING, 199, TEXT("&Option"));  
	::AppendMenu(m_hMenu, MF_SEPARATOR, 0, 0);  
	::AppendMenu(m_hMenu, MF_STRING, 200, TEXT("&Update"));  
	::AppendMenu(m_hMenu, MF_SEPARATOR, 0, 0);  
	::AppendMenu(m_hMenu, MF_STRING, 201, TEXT("&Close")); 
}  

CDeskBandExt::~CDeskBandExt()  
{  
	ATLTRACE("CDeskBandExt::~CDeskBandExt");  
}  

HRESULT CDeskBandExt::FinalConstruct()  
{  
	ATLTRACE("CDeskBandExt::FinalConstruct");  

	m_pSite = NULL;  
	m_nBandID = 0;  
	m_bFocus = FALSE;  
	m_hwndParent = NULL;  
	m_dwViewMode = DBIF_VIEWMODE_NORMAL;  
	m_bCompositionEnabled = TRUE;  

	
	return S_OK;  
}  

void CDeskBandExt::FinalRelease()  
{
	if ( m_pSite )  
		m_pSite->Release();  

	m_pSite = NULL;  
}  

STDMETHODIMP CDeskBandExt::SetSite(  
	IUnknown *pUnkSite)  
{  
	if ( m_pSite )  
		m_pSite->Release();  

	m_pSite = NULL;  

	if ( !pUnkSite )  
		return E_FAIL;  

	CComQIPtr<IOleWindow> pOleWindow;  

	m_hwndParent = NULL;  

	if ( SUCCEEDED(pUnkSite->QueryInterface(IID_IOleWindow, (LPVOID*)&pOleWindow)) )  
		pOleWindow->GetWindow(&m_hwndParent);  

	if ( !m_hwndParent )  
		return E_FAIL;  

	if ( !IsWindow() )  
		Create(m_hwndParent);  

	if ( !IsWindow() )  
		return E_FAIL;  

	pUnkSite->QueryInterface(IID_IInputObjectSite, (LPVOID*)&m_pSite);  

	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::GetSite(  
	REFIID riid,   
	LPVOID *ppvReturn)  
{  
	*ppvReturn = NULL;  

	if ( m_pSite )  
		return m_pSite->QueryInterface(riid, ppvReturn);  

	return E_FAIL;  
}  

STDMETHODIMP CDeskBandExt::UIActivateIO(  
	BOOL fActivate,   
	PMSG)  
{  
	if ( fActivate )  
		SetFocus();  
	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::HasFocusIO()  
{  
	ATLTRACE("CDeskBandExt::HasFocusIO");  

	return (m_bFocus ? S_OK : S_FALSE);  
}  

STDMETHODIMP CDeskBandExt::TranslateAcceleratorIO(  
	PMSG)  
{  
	ATLTRACE("CDeskBandExt::TranslateAcceleratorIO");  

	return S_FALSE;  
}  

STDMETHODIMP CDeskBandExt::GetBandInfo(  
	DWORD dwBandID,  
	DWORD dwViewMode,  
	DESKBANDINFO *pdbi)  
{  
	if ( !pdbi )   
		return E_INVALIDARG;  

	m_nBandID = dwBandID;  
	m_dwViewMode = dwViewMode;  

	if ( pdbi->dwMask & DBIM_MODEFLAGS )  
		pdbi->dwModeFlags = DBIMF_VARIABLEHEIGHT;  

	if ( pdbi->dwMask & DBIM_MINSIZE )  
		pdbi->ptMinSize = CalcIdealSize();  

	if ( pdbi->dwMask & DBIM_MAXSIZE )  
	{  
		pdbi->ptMaxSize.x = -1;  
		pdbi->ptMaxSize.y = -1;  
	}  

	if ( pdbi->dwMask & DBIM_INTEGRAL )  
	{  
		pdbi->ptIntegral.x = 1;  
		pdbi->ptIntegral.y = 1;  
	}  

	if ( pdbi->dwMask & DBIM_ACTUAL )  
		pdbi->ptActual = CalcIdealSize();  

	if ( pdbi->dwMask & DBIM_TITLE )  
	{  
		if ( dwViewMode == DBIF_VIEWMODE_FLOATING )  
		{  
			lstrcpynW(pdbi->wszTitle, CT2W(m_lastString), ARRAYSIZE(pdbi->wszTitle));  
		}  
		else  
		{  
			pdbi->dwMask &= ~DBIM_TITLE;  
		}  
	}  

	if ( pdbi->dwMask & DBIM_BKCOLOR )  
		pdbi->dwMask &= ~DBIM_BKCOLOR;  

	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::GetClassID(  
	LPCLSID pClassID)  
{  
	ATLTRACE("CDeskBandExt::GetClassID");  

	*pClassID = CLSID_CDeskBandExt;  
	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::IsDirty(void)  
{  
	ATLTRACE("CDeskBandExt::IsDirty");  

	return S_FALSE;  
}  

STDMETHODIMP CDeskBandExt::Load(  
	LPSTREAM pStream)  
{  
	ATLTRACE("CDeskBandExt::Load");  

	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::Save(  
	LPSTREAM pStream,   
	BOOL fClearDirty)  
{  
	ATLTRACE("CDeskBandExt::Save");  

	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::GetSizeMax(  
	ULARGE_INTEGER *pul)  
{  
	ATLTRACE("CDeskBandExt::GetSizeMax");  

	return E_NOTIMPL;  
}  

STDMETHODIMP CDeskBandExt::CanRenderComposited(  
	BOOL *pfCanRenderComposited)  
{  
	ATLTRACE("CDeskBandExt::CanRenderComposited");  

	if ( pfCanRenderComposited )  
		*pfCanRenderComposited = TRUE;  
	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::SetCompositionState(  
	BOOL fCompositionEnabled)  
{  
	ATLTRACE("CDeskBandExt::SetCompositionState");  

	m_bCompositionEnabled = fCompositionEnabled;  
	Invalidate();  
	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::GetCompositionState(  
	BOOL *pfCompositionEnabled)  
{  
	ATLTRACE("CDeskBandExt::GetCompositionState");  

	if ( pfCompositionEnabled )  
		*pfCompositionEnabled = m_bCompositionEnabled;  
	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::GetWindow(  
	HWND *phwnd)  
{  
	ATLTRACE("CDeskBandExt::GetWindow");  

	*phwnd = m_hWnd;  
	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::ContextSensitiveHelp(  
	BOOL)  
{  
	ATLTRACENOTIMPL("IOleWindow::ContextSensitiveHelp");  

	return E_NOTIMPL;  
}  

STDMETHODIMP CDeskBandExt::ShowDW(  
	BOOL fShow)  
{  
	ATLTRACE("CDeskBandExt::ContextSensitiveHelp");  

	if ( IsWindow() )  
		ShowWindow(fShow ? SW_SHOW : SW_HIDE);  

	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::CloseDW(  
	DWORD)  
{  
	ATLTRACE("CDeskBandExt::CloseDW");  

	ShowDW(FALSE);  

	if ( IsWindow() )  
		DestroyWindow();  

	m_hWnd = NULL;  

	return S_OK;  
}  

STDMETHODIMP CDeskBandExt::ResizeBorderDW(  
	LPCRECT prcBorder,  
	IUnknown *punkToolbarSite,  
	BOOL)  
{  
	ATLTRACE("CDeskBandExt::ResizeBorderDW");  

	if ( !IsWindow() )   
		return S_OK;  

	if ( punkToolbarSite )  
	{  
		BORDERWIDTHS bw = { 0 };  
		bw.top = bw.bottom = ::GetSystemMetrics(SM_CYBORDER);  
		bw.left = bw.right = ::GetSystemMetrics(SM_CXBORDER);  

		CComQIPtr<IDockingWindowSite> pDock = punkToolbarSite;  

		if ( SUCCEEDED(pDock->RequestBorderSpaceDW(static_cast<IDeskBand*>(this), &bw)) )  
		{  
			if ( SUCCEEDED(pDock->SetBorderSpaceDW(static_cast<IDeskBand*>(this), &bw)) )  
			{  
				MoveWindow(prcBorder);  
				return S_OK;  
			}  
		}  
	}  

	return E_FAIL;  
}  

STDMETHODIMP CDeskBandExt::QueryContextMenu(   
	HMENU hMenu,  
	UINT indexMenu,  
	UINT idCmdFirst,  
	UINT idCmdLast,  
	UINT uFlags)  
{  
	ATLTRACE("CDeskBandExt::QueryContextMenu");  

	if ( !(CMF_DEFAULTONLY & uFlags) )  
	{  
		InsertMenu(hMenu, indexMenu, MF_STRING | MF_BYPOSITION, idCmdFirst + IDM_COMMAND, TEXT("&Desk Band Command"));  
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(IDM_COMMAND + 1));  
	}  

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);  
}  

STDMETHODIMP CDeskBandExt::InvokeCommand(  
	LPCMINVOKECOMMANDINFO lpcmi)  
{  
	ATLTRACE("CDeskBandExt::InvokeCommand");  

	switch ( LOWORD(lpcmi->lpVerb) )  
	{  
	case IDM_COMMAND:  
		break;  
	default:  
		return E_INVALIDARG;  
	}  

	return NOERROR;  
}  

STDMETHODIMP CDeskBandExt::GetCommandString(   
	UINT_PTR idCommand,  
	UINT uFlags,  
	LPUINT lpReserved,  
	LPSTR lpszName,  
	UINT uMaxNameLen)  
{  
	ATLTRACE("CDeskBandExt::GetCommandString");  

	HRESULT  hr = E_INVALIDARG;  

	switch(uFlags)  
	{  
	case GCS_HELPTEXT:  
		switch(idCommand)  
		{  
		case IDM_COMMAND:  
			lstrcpynA(lpszName, "this is a sample menu for IDeskBand study.", uMaxNameLen);  
			hr = NOERROR;  
			break;  
		}  
		break;  
	case GCS_VERB:  
		switch(idCommand)  
		{  
		case IDM_COMMAND:  
			lstrcpynA(lpszName, "command", uMaxNameLen);  
			hr = NOERROR;  
			break;  
		}  
		break;  
	case GCS_VALIDATE:  
		hr = NOERROR;  
		break;  
	}  

	return hr;  
}  

LRESULT CDeskBandExt::OnBuilderCallBack(IDuiObject* pObj, int nCallBack)
{
	if ( pObj->QueryInterface(IIDuiHostWindowCtrl) && nCallBack == BuilderCallBack_GetHostWindow)
	{
		return (LRESULT)m_hWnd;
	}

	return NULL;
}

LRESULT CDeskBandExt::OnCreate(  
	UINT /*uMsg*/,  
	WPARAM /*wParam*/,  
	LPARAM /*lParam*/,  
	BOOL& bHandled)  
{  
	bHandled = TRUE;
	ATLTRACE("CDeskBandExt::OnCreate");  
	SetWindowText(L"jc_desk_band");
	ShowDW(TRUE);
	SetTimer(UPDATE_TIMER_ID, 5000);  

	//MessageBox(L"tiaoshi");

	//DuiKit

	m_pDuiCore = CreateObject(OBJECT_CORE);
	if ( !m_pDuiCore )
	{
		return FALSE;
	}



	m_pDuiSkin = CreateObject(OBJECT_CORE_SKIN_FILE);
	if ( !m_pDuiSkin )
	{
		return FALSE;
	}
	m_pDuiSkin->SetName(L"mainframe");
	m_pDuiSkin->SetCore(m_pDuiCore);


	CDuiString sFileSkin;
	GetModuleFileName(g_instance, sFileSkin.GetBufferSetLength(MAX_PATH), MAX_PATH);
	sFileSkin.ReleaseBuffer();
	sFileSkin = sFileSkin.Left(sFileSkin.ReverseFind('\\'));
	sFileSkin.Append(_T("\\..\\skin\\deskband_skin\\"));
	m_pDuiSkin->Open(sFileSkin.GetBuffer(), sFileSkin.Length());

	CDuiObjectPtr<IDuiBuilder, IIDuiBuilder> pBuild = CreateObject(OBJECT_CORE_BUILD);
	pBuild->BuildAsFile(_T("main.xml"), m_pDuiSkin, NULL,this);


	m_pHostWnd = pBuild->GetRootControl()->QueryInterface(IIDuiWindowCtrl);
	//m_pHostWnd->Subclass(m_hWnd);

	return 0L;  
}  

LRESULT CDeskBandExt::OnSize(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnSize");  

// 	typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags); 
// 	lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes; 
// 
// 
// 	typedef BOOL (WINAPI *MYFUNC)(HWND,COLORREF,BYTE,DWORD); 
// 	COLORREF maskColor=RGB(0X80,0XC0,0XFF); 
// 	HMODULE hUser32 = GetModuleHandle(L"user32.dll"); 
// 	SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32,"SetLayeredWindowAttributes"); 
// 	::SetWindowLong(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE)|WS_EX_LAYERED); 
// 	SetLayeredWindowAttributes(m_hWnd, maskColor, 255, LWA_COLORKEY); 
// 	FreeLibrary(hUser32);


	return 0;  
}  

LRESULT CDeskBandExt::OnCommand(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnCommand");  

	if ( wParam == 201 )  
	{  
		CComPtr<IBandSite> spBandSite;  

		//if ( SUCCEEDED(spBandSite.CoCreateInstance(CLSID_TrayBandSiteService)) )  
		//  spBandSite->RemoveBand(m_nBandID);  
	}  

	return 0;  
}  

LRESULT CDeskBandExt::OnRButtonUp(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnRButtonUp");  

	POINT pos;  

	::GetCursorPos(&pos);  
	::TrackPopupMenuEx(m_hMenu, TPM_LEFTBUTTON, pos.x, pos.y, m_hWnd, NULL);  

	return 0;  
}  

LRESULT CDeskBandExt::OnLButtonUp(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnLButtonUp");  
	
	return 0;  
}  

LRESULT CDeskBandExt::OnMouseMove(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnMouseMove");  

	return 0;  
}  

LRESULT CDeskBandExt::OnMouseLeave(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnMouseLeave");  

	return 0;  
}  

LRESULT CDeskBandExt::OnMouseHover(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnMouseHover");  

	return 0;  
}  

LRESULT CDeskBandExt::OnDestroy(  
	UINT uMsg,  
	WPARAM /*wParam*/,  
	LPARAM /*lParam*/,  
	BOOL& bHandled)  
{  
	ATLTRACE("CDeskBandExt::OnDestroy");  
	KillTimer(UPDATE_TIMER_ID);  

	if ( m_hMenu )  
		::DestroyMenu(m_hMenu);  
	m_hMenu = NULL;  
	m_hWnd = NULL;  
	return 0L;  
}  

LRESULT CDeskBandExt::OnFocus(  
	UINT uMsg,  
	WPARAM /*wParam*/,  
	LPARAM /*lParam*/,  
	BOOL& /*bHandled*/)  
{  
	ATLTRACE("CDeskBandExt::OnFocus");  

	m_bFocus = (uMsg == WM_SETFOCUS);  
	m_pSite->OnFocusChangeIS((IDockingWindow*)this, m_bFocus);  
	return 0L;  
}  

LRESULT CDeskBandExt::OnEraseBackground(  
	UINT /*uMsg*/,  
	WPARAM /*wParam*/,  
	LPARAM /*lParam*/,  
	BOOL& /*bHandled*/)  
{  
	return 1;  
}  


void CDeskBandExt::OnFinalMessage( HWND hWnd )
{
}



POINTL CDeskBandExt::CalcIdealSize() const  
{  
	ATLTRACE("CDeskBandExt::CalcIdealSize");  

	POINTL pt;  

	if ( !IsWindow() )  
	{  
		pt.x = ::GetSystemMetrics(SM_CXMIN);  
		pt.y = ::GetSystemMetrics(SM_CYMIN);  
	}  
	else  
	{  
		CString meas;  
		SIZE size = { 150,150 };  
// 		HDC dc = CreateIC(_T("DISPLAY"), NULL, NULL, NULL);  
// 		HFONT hFont = (HFONT)::SelectObject(dc,  m_pStyle->GetFont());  
// 
// 		meas.Empty();  
// 		for ( int i = 0; i < 12; i++ )  
// 			meas += " ";  
// 		::GetTextExtentPoint32(dc, meas, meas.GetLength(), &size);  
// 		::SelectObject(dc, hFont);  
// 
// 		::DeleteDC(dc);  
		pt.x = size.cx + ::GetSystemMetrics(SM_CXSMSIZE) + 5;  
		pt.y = ::GetSystemMetrics(SM_CYSMSIZE);  
	}  

	return pt;  
}  
#define RED_COLOR RGB(255,0,0)
#define BLUE_COLOR RGB(0,0,255)

#include <uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
LRESULT CDeskBandExt::OnPaint(  
	UINT uMsg,  
	WPARAM wParam,  
	LPARAM lParam,  
	BOOL& bHandled)  
{ 
	bHandled = FALSE;
	return 0;  
}  

LRESULT CDeskBandExt::OnTimer(  
	UINT /*uMsg*/,  
	WPARAM /*wParam*/,  
	LPARAM /*lParam*/,  
	BOOL& /*bHandled*/)  
{  
	//ATLTRACE("CDeskBandExt::OnTimer");  
	SYSTEMTIME st;  

// 	::GetSystemTime(&st);  
// 
// 	m_lastString = m_Ss.Mid(m_sl, 5);  
// 	if ( ++m_sl >= (m_Ss.GetLength() - 5) )  
// 		m_sl = 0;  

	static BOOL bShow = TRUE;
	if ( bShow )
	{
		ShowDW(FALSE);
		bShow = FALSE;
	}
	else
	{
		ShowDW(TRUE);
		bShow = TRUE;
	}

	
	
	Invalidate();  

	return 0;  
}  

LRESULT CDeskBandExt::OnPowerBroadcast(  
	UINT /*uMsg*/,  
	WPARAM wParam,  
	LPARAM /*lParam*/,  
	BOOL& /*bHandled*/)  
{  
	ATLTRACE("CDeskBandExt::OnPowerBroadcast");  

	if ( wParam == PBT_APMRESUMECRITICAL ||  
		wParam == PBT_APMRESUMESUSPEND ||  
		wParam == PBT_APMRESUMESTANDBY )  
		SetTimer(UPDATE_TIMER_ID, 500);  
	return 0;  
}  

LRESULT CDeskBandExt::OnThemeChanged(  
	UINT /*uMsg*/,  
	WPARAM /*wParam*/,  
	LPARAM /*lParam*/,  
	BOOL& /*bHandled*/)  
{  
	ATLTRACE("CDeskBandExt::OnThemeChanged");  

	return 0;  
}  