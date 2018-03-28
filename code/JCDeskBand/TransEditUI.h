/*
* Copyright (c) 2014 Lourking
* All rights reserved.
* 
* 文件名称：d:\WorkSpace\
* 文件标识：
* 摘    要：
* 
* 当前版本：1.0
* 作    者：Lourking
* 完成日期：
*
* 取代版本：
* 原作者  ：Lourking
* 完成日期：
*/
//---------------------------------------------------------------------------------------------------------------

#ifndef __DSTRANSEDIT_H__
#define __DSTRANSEDIT_H__

#include <atlbase.h>
#include <atlapp.h>
#include <atlbase.h>
#include <atlcom.h>
//#include <atlctl.h>
#include <atlmisc.h>
#include <WindowsX.h>

#include <atlctrls.h>

#include <Richole.h>

const int ITEM_HEIGHT = 24;

static BOOL SetRealTimePriority()
{
	if( ! SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS ))
	{
		return false;
	}
	return true;
}



class IRichEditOleCallbackEx: public IRichEditOleCallback
{
public:
	IRichEditOleCallbackEx() :pStorage(NULL),m_iNumStorages(0),m_dwRef(0)
	{
		m_hEditWnd = NULL;				
		HRESULT hResult = ::StgCreateDocfile(NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE /*| STGM_DELETEONRELEASE */|STGM_CREATE , 0, &pStorage );

		if (hResult != S_OK )
		{

		}
	}

	virtual HRESULT STDMETHODCALLTYPE GetNewStorage(LPSTORAGE* lplpstg)
	{
		WCHAR tName[50];
		wsprintfW(tName, L"REOLEStorage%d", m_iNumStorages++);

		HRESULT hResult = pStorage->CreateStorage(tName,
			STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE ,
			0, 0, lplpstg );

		if (hResult != S_OK )
		{

		}		    
		return hResult;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
	{
		HRESULT hr = S_OK;
		*ppvObject = NULL;

		if ( iid == IID_IUnknown ||
			iid == IID_IRichEditOleCallback )
		{
			*ppvObject = this;
			AddRef();
			hr = NOERROR;
		}
		else
		{
			hr = E_NOINTERFACE;
		}

		return hr;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++m_dwRef;
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		if ( --m_dwRef == 0 )
		{
			delete this;			
			return 0;
		}

		return m_dwRef;
	}

	virtual HRESULT STDMETHODCALLTYPE GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
		LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ShowContainerUI(BOOL fShow)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DeleteObject(LPOLEOBJECT lpoleobj)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
		DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode)
	{
		return S_OK;
	}
	 

	virtual HRESULT STDMETHODCALLTYPE GetClipboardData(CHARRANGE FAR * lpchrg, DWORD reco, LPDATAOBJECT FAR * lplpdataobj)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg, HMENU FAR *lphmenu)
	{
		//HCURSOR hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		//::SetCursor(hCursor);

		HMENU hMenu = CreatePopupMenu();
		GETTEXTLENGTHEX stuGlt;
		stuGlt.flags = GTL_NUMBYTES;
		stuGlt.codepage = CP_ACP;
		int nResult = SendMessage(m_hEditWnd, EM_GETTEXTLENGTHEX, (WPARAM)&stuGlt, 0);
		if ( !nResult)
		{
			AppendMenu(hMenu, MF_STRING|MF_DISABLED|MF_GRAYED, 1, _T("剪切(&T)"));
			AppendMenu(hMenu, MF_STRING|MF_DISABLED|MF_GRAYED, 2, _T("复制(&C)"));
		}
		else
		{
			AppendMenu(hMenu, MF_STRING, 1, _T("剪切(&T)"));			
			AppendMenu(hMenu, MF_STRING, 2, _T("复制(&C)"));
		}
		AppendMenu(hMenu, MF_STRING, 3, _T("粘贴(&P)"));
		AppendMenu(hMenu, MF_SEPARATOR, 4, _T(""));
		AppendMenu(hMenu, MF_STRING, 5, _T("缩回图标(&S)"));	
		AppendMenu(hMenu, MF_STRING, 6, _T("关于魔框(&A)"));
		AppendMenu(hMenu, MF_STRING, 7, _T("反馈建议(&F)"));
		AppendMenu(hMenu, MF_STRING, 8, _T("退出(&E)"));					
				
		*lphmenu = hMenu;

		POINT pt;
		GetCursorPos(&pt);
		int nRet = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_TOPALIGN|TPM_RETURNCMD, 
			pt.x, pt.y, 0, m_hEditWnd/*::GetParent(m_hEditWnd)*/, NULL);

		switch(nRet)
		{
		case 1:
			::SendMessage(m_hEditWnd, WM_CUT, 0, 0);
			break;
		case 2:
			::SendMessage(m_hEditWnd, WM_COPY, 0, 0);
			break;

		case 3:
			::SendMessage(m_hEditWnd, WM_PASTE, 0, 0);
			break;
		
		case 5:
			::SendMessage(::GetParent(m_hEditWnd), MG_MSG_SHRINK, 0, 0);
			break;

		case 6:
			{
				//::SendMessage(m_hEditWnd, WM_PASTE, 0, 0L);
				//
				//TCHAR sz[MAX_PATH];
				//::GetWindowText(m_hEditWnd, sz, MAX_PATH);				
				// //SendMessage(::GetParent(m_hEditWnd), WM_NAVIGATE, 0, (LPARAM)sz);

				::SendMessage(::GetParent(m_hEditWnd), WM_COMMAND, IDC_BTN_ABOUT, 0);

			}
			break;
		case 7:
			{							
				CString strUrl = _T("http://so.languang.com/feedback.htm");
				int nRet = OpenUrlByDefaultBrowser(strUrl);
				if (nRet == 0)
				{
					ShellExecute(NULL, L"open", strUrl, NULL, NULL, SW_SHOW);
				}
			
			}
			break;
		case 8:
			{
				::SendMessage(::GetParent(m_hEditWnd), MG_MSG_EXIT, 0, 0);
			}
			break;
		}		

		::PostMessage(::GetParent(m_hEditWnd), WM_CANCELMODE, 0, 0);

		return S_OK;					

	}

	void SetEditWnd(HWND hEditWnd){ m_hEditWnd = hEditWnd; 	}

private:

	int m_iNumStorages;
	IStorage* pStorage;
	DWORD m_dwRef;

	HWND m_hEditWnd;



};

class CTransEditUI :
	public CWindowImpl<CTransEditUI, CRichEditCtrl>,
	public CLabelUI
{
public:	

	DECLARE_DUICONTROL(CTransEditUI)

	LPCTSTR GetClass() const
	{
		return _T("TransEdit" );
	}

	CTransEditUI()
	{
		m_nOldLen  = -1;
		m_hListBox = NULL;
		m_bIgnoreChange = FALSE;
		m_bLBtnDown = FALSE;
		m_bBack = FALSE;
		m_hBitmap = NULL;
		m_bIgnoreUpdate = FALSE;
	}

	~CTransEditUI() {

		::DeleteObject(m_hBitmap);
	}

public: 
	DECLARE_WND_CLASS(_T("CTransEditUI"))
	//DECLARE_WND_CLASS_EX(_T("CTransEditUI"), CS_DBLCLKS, (HBRUSH)::GetStockObject(WHITE_BRUSH))	
	
	BEGIN_MSG_MAP(CTransEditUI)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(MG_MSG_IGNORE_ONE_CHANGE, OnIgnoreOneChange)
				
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		//MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CHAR(OnChar)
		MSG_WM_CUT(OnCut)
		MSG_WM_COPY(OnCopy)			
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
		MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)				

		MESSAGE_HANDLER(WM_CTLCOLOREDIT , OnCtlColor)
		//MESSAGE_HANDLER(EM_SETCHARFORMAT, OnSetCharFormat)
		 
		//MESSAGE_HANDLER(EM_EXSETSEL, OnExSetSel)
		//MSG_WM_COMMAND(OnCommand)
		//MESSAGE_HANDLER(EM_SETSEL, OnSetSel)
		//MESSAGE_HANDLER(WM_PASTE, OnPaste)
		/*MESSAGE_HANDLER(wm_pa)*/

		//REFLECTED_COMMAND_CODE_HANDLER(ENM_CHANGE, OnEditChange)
		//COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChange)
	END_MSG_MAP()		
	
	void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl){

		return;
	}

	LRESULT OnSetSel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/){


		return 0;
	}

	LRESULT OnExSetSel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled/**/)
	{
		bHandled = TRUE;	
		return 0;
	}

	void OnPaint(CDCHandle dc)
	{											
		ATLTRACE(_T("CTransEditUI:OnPaint"));
		static BOOL bFocused = FALSE;

		if (m_bIgnoreUpdate)
		{
			m_bIgnoreUpdate = FALSE;

			if (bFocused)
			{				
				bFocused = FALSE;
				CopyWndToBitmap(GetParent(), m_hBitmap);
			}
		}
		else
		{
			CopyWndToBitmap(GetParent(), m_hBitmap);
		}		
	
		CRect rtParnet, rtThis;
		GetParent().GetWindowRect(&rtParnet);
		GetWindowRect(&rtThis);

		int nLeft = abs(rtThis.left - rtParnet.left);
		int nTop  = abs(rtThis.top  - rtParnet.top);

		CPaintDC dcPaint(*this);		 		 
		CRect rt;
		GetClientRect(&rt);

		CMemoryDC dcMem(dcPaint, rt);
		dcMem.SetBkMode(TRANSPARENT);

		if (m_hBitmap)
		{
			CDC dcBitmap;
			dcBitmap.CreateCompatibleDC(dcMem);		

			HBITMAP hOldBitmap = (HBITMAP)dcBitmap.SelectBitmap(m_hBitmap);					
			dcMem.BitBlt(0, 0, rt.Width(), rt.Height(), dcBitmap, nLeft, nTop, SRCCOPY);			
			dcBitmap.SelectBitmap(hOldBitmap);						

		}

		TCHAR szText[MAX_PATH];
		GetWindowText(szText, MAX_PATH);

		//-----------------------------------------------------------------------------------
		//Special Handler for tab key:
		//-----------------------------------------------------------------------------------
		if (GetFocus() == m_hWnd)
		{			
			LONG nStart, nEnd;
			GetSel(nStart, nEnd);

			if (nStart >= 0 && nEnd > nStart)
			{	
				CString strVal(szText);
				strVal.TrimLeft();
				strVal.TrimRight();

				if (strVal.GetLength() == 0)
				{
					CSize sz;
					dcMem.GetTextExtent(_T(" "), 1, &sz);

					CPoint pt = PosFromChar(nEnd);						
					CRect rtText;

					if (OsIsXpOrEarlier())
					{
						rtText.SetRect(	
							0,										
							0, //rt.CenterPoint().y - _size.cy / 2,  
							pt.x,
							sz.cy  //rt.CenterPoint().y - _size.cy / 2 + _size.cy - 1
							);

						//rt.top = rtText.top;
						//rt.bottom = rtText.bottom;				
					}
					else
					{
						rtText.SetRect(
							0,										
							rt.CenterPoint().y - sz.cy / 2,  
							pt.x,
							rt.CenterPoint().y - sz.cy / 2 + sz.cy - 2
							);
					}

					dcMem.FillSolidRect(&rtText, RGB(57, 57, 255));		
					return;

				}				

			}

		}

		//-----------------------------------------------------------------------------------

		CString strText(szText);
		ATLTRACE(_T("Len:%d"), strText.GetLength());

		if (strText.GetLength() > 0)
		{				
			HFONT hOldFont = dcMem.SelectFont(m_hRegularFont);

			LONG nStart, nEnd;
			GetSel(nStart, nEnd);

			if (GetFocus() == m_hWnd)
			{					
				if (nStart >= 0 && nEnd > nStart)
				{							
					CString str(szText);
					str = str.Mid(nStart, nEnd - nStart);

					int nSelect_X = 0;

					SIZE _size;
					dcMem.GetTextExtent(str, str.GetLength(), &_size);
					nSelect_X = _size.cx;

					int nLeft = 0;					
					if (nStart >= 1)
					{
						/*	
						str = szText;
						str = str.Mid(0, nStart);
						dcMem.GetTextExtent(str, str.GetLength(), &_size);
						nLeft = _size.cx;

						CSize sz;
						dcMem.GetTextExtent(strText, strText.GetLength(), &sz);
						if (sz.cx > rt.Width())
						{
						nLeft -= abs(sz.cx - rt.Width());
						}

						*/
						CPoint pt = PosFromChar(nStart);			
						nLeft = pt.x;
						ATLTRACE(_T("nLeft:%d"), nLeft);							

					}

					CRect rtText;

					if (OsIsXpOrEarlier())
					{
						rtText.SetRect(	
							nLeft,										
							0, //rt.CenterPoint().y - _size.cy / 2,  
							nLeft + nSelect_X,
							_size.cy  //rt.CenterPoint().y - _size.cy / 2 + _size.cy - 1
							);

						//rt.top = rtText.top;
						//rt.bottom = rtText.bottom;				
					}
					else
					{
						rtText.SetRect(
							nLeft,										
							rt.CenterPoint().y - _size.cy / 2,  
							nLeft + nSelect_X,
							rt.CenterPoint().y - _size.cy / 2 + _size.cy - 2
							);
					}

					ATLTRACE(_T("rtText:%d--%d--%d--%d"), rtText.left, rtText.top, rtText.right, rtText.bottom);												
					dcMem.FillSolidRect(&rtText, RGB(57, 57, 255));		
					bFocused = TRUE;

				}

			}				

			dcMem.SetTextColor(RGB(255, 255, 255));	

			if (OsIsXpOrEarlier())
			{
				rt.OffsetRect(0, -4);
			}

			//dcMem.DrawText(szText, _tclen(szText), &rt, DT_LEFT);		

			CSize sz;
			dcMem.GetTextExtent(strText, strText.GetLength(), &sz);
			if (sz.cx > rt.Width())
			{
				/*
				int nX     = sz.cx / strText.GetLength();  
				int nCount = rt.Width() / nX;     
				int nCaret = nEnd + 1;					
				int nBegin = strText.GetLength() - nCount;
				strText = strText.Mid(nBegin);
				*/

				//if (nStart == nEnd)
				{
					if (nEnd == strText.GetLength())
					{
						CPoint pt = PosFromChar(nEnd);			
						ATLTRACE(_T("PosFromChar:%d--%d"), pt.x, pt.y);
						rt.right = pt.x;

						dcMem.DrawText(szText, _tcslen(szText), &rt, DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
						return;
					}
					else if (nEnd < strText.GetLength())
					{						
						CPoint pt = PosFromChar(nEnd);				//						
						CRect rtLeft(rt);
						rtLeft.right = pt.x;

						CRect rtRight(rt);
						rtRight.left = pt.x;

						CString strLeft  = strText.Mid(0, nEnd);
						CString strRight = strText.Mid(nEnd);

						ATLTRACE(_T("nEnd < strText.GetLength() strLeft:%s strRight:%s"), strLeft, strRight);

						dcMem.DrawText(strLeft, strLeft.GetLength(), rtLeft, DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
						dcMem.DrawText(strRight, strRight.GetLength(), rtRight, DT_LEFT|DT_SINGLELINE|DT_VCENTER);									
						return;


					}

				}		

			}

			//dcMem.TextOut(0, rt.CenterPoint().y - m_TextMetrix.cy / 2, strText);			
			//dcMem.DrawTextEx(szText, _tcslen(szText), &rt, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
					
			dcMem.DrawText(szText, _tcslen(szText), &rt, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
			dcMem.SelectFont(hOldFont);
			ATLTRACE(_T("Start:%d, End:%d Caret Index:%d"), nStart, nEnd, nEnd + 1);

		}	

	}

	void OnSize(UINT nType, CSize size)
	{
		SetMsgHandled(FALSE);		

	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		 if (nIDEvent == 1001)
		 {
			 if (!::IsWindowVisible(g_hThumbWnd))
			 {
				 ::PostMessage(GetParent(), MG_MSG_DISADAPTERDLG, 0, 0);
			 }
			 
			 KillTimer(1001);
		 }

		 else if (nIDEvent == 1002)
		 {
			 KillTimer(1002);
			 RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		 }
		
	}

	LRESULT OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/){

		
		BOOL bRet = OpenClipboard();
		if ( bRet ){
			long nMin,nMax;
			GetSel(nMin, nMax);


			HANDLE hClip = GetClipboardData(CF_TEXT);

			LPVOID pbuf = GlobalLock(hClip);

			char *pszBuf = (char*)pbuf;

			int nTextLen = strlen(pszBuf);


			DefWindowProc();

			GlobalUnlock(hClip);

			int nRange = nMax - nMin;

			//SetSel(nMin + nTextLen, nMin + nTextLen);
			SetSel(1,2);

		}else
			DefWindowProc();

		

		return 0;
	}

	
	

protected:
	LRESULT OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DWORD dwID = (DWORD)wParam;
		switch( dwID )
		{
		case MG_MSG_COPY:
			SendMessage(WM_COPY, 0, 0);
			break;
		case MG_MSG_PASTE:
			SendMessage(WM_PASTE, 0, 0);
			break;
		case MG_MSG_CUT:
			SendMessage(WM_CUT, 0, 0);
			break;
		}
		SetMsgHandled(FALSE);
		return 0;
	}

	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{

		return 1;
	}

	LRESULT OnSetCharFormat(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = 0;
		return 0;
	}


	void OnCopy()
	{

		BOOL bRet = OpenClipboard();
		if ( bRet )
		{
			
			CHARRANGE cr;
			cr.cpMin = cr.cpMax = 0;
			::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
			CString strText;

			if(cr.cpMax > cr.cpMin)
			{
				::EmptyClipboard();

				LPTSTR lpsz=strText.GetBufferSetLength((cr.cpMax - cr.cpMin + 1) * 2);
				lpsz[0] = NULL;
				::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpsz);
				strText.ReleaseBuffer();

				int nLen = strText.GetLength()+2;
				HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, nLen*sizeof(TCHAR));
				LPTSTR lpwstr = (LPTSTR)GlobalLock(hglb);
				ZeroMemory(lpwstr,nLen*sizeof(TCHAR));
				_tcscpy_s(lpwstr, nLen, strText);

				GlobalUnlock(hglb); 

				SetClipboardData(CF_UNICODETEXT, hglb);
			}
					
		}

		
		CloseClipboard();
	}
	void OnCut()
	{
		if(0 == GetTextLength())
			return;

		OnCopy();
		SetMsgHandled(FALSE);
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0; 
	}

	LRESULT OnIgnoreOneChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetIgnore(TRUE);
		return 0; 
	}
	

	void OnKeyDown(TCHAR key, UINT repeat_count, UINT flags)
	{
		SetMsgHandled(FALSE);					
		m_bIgnoreUpdate = TRUE;				
						
		//---------------------------------------------------------
		//Tough Added:
		if (::IsWindowVisible(g_hThumbWnd))
		{
			::PostMessage(g_hThumbWnd, MG_MSG_HIDEPOPUP, 0, 0);				
		}				
		//---------------------------------------------------------

		if (key == VK_RETURN )  /* && ::GetFocus() == m_hWnd*/
		{
			if(!IsWindowVisible())
				return;

			WCHAR szWord[128];
			GetWindowText(szWord, 128);			
			
			WCHAR szUrl[256];

			CString strUrl;

			if(ListView_UrlMatch::getInstance()->GetSelectedUrl(strUrl)){
				HistoryAdapter::getInstance()->Add(strUrl);
				UrlAdapterMgr::getInstance()->AddSelfHistoryUrl(strUrl);

				if(0 != strUrl.Find(_szHttpHead2 ))
					strUrl.Insert(0, _szHttpHead2);

				UrlNavigateMgr::getInstance()->ExchangeOnTn(strUrl);				
				

			}
			else if(UrlFilter::getInstance()->Filter(szWord))
			{
				HistoryAdapter::getInstance()->Add(szWord);
				UrlAdapterMgr::getInstance()->AddSelfHistoryUrl(CString(szWord));

				strUrl = szWord;

				if(0 != strUrl.Find(_szHttpHead2 ))
					strUrl.Insert(0, _szHttpHead2);	

				UrlNavigateMgr::getInstance()->ExchangeOnTn(strUrl);
			
			}
			else
			{
				strUrl = ConfigMgr::getInstance()->GetSearchEngine()->MakeSearchDstUrl(szWord);				
			}		

			if (
				strUrl.Find(_T("baidu.com")) != -1 && 
				strUrl.Find('?') == -1
				)			
			{

				CString strTn = SearchEngineMgr::getInstance()->m_vecSE[0].strSearchParamFormat;
				int nLeft  = strTn.Find(_T("tn="));
				int nRight = strTn.Find(_T("&"), nLeft);
				nRight = (nRight == -1) ? strTn.GetLength() : nRight;				
				strTn = strTn.Mid(nLeft, nRight - nLeft);

				if (strTn.IsEmpty())
				{
					strUrl = _T("http://www.baidu.com/?tn=02049043_13_pg");				
				}
				else
				{
					strUrl.Format(_T("http://www.baidu.com/?%s"), strTn);
				}
				
			}

			//------------------------------------------------------
			//Tough Added:
			ConfigMgr::getInstance()->HidePopopWnd();	
			::SetForegroundWindow(::GetDesktopWindow());
			//-------------------------------------------------------
			
			//----------------------------------------------------------------------
			//2015 3.24 Tough Added:
			
			int nRet = OpenUrlByDefaultBrowser(strUrl);
			if (nRet == 0)
			{
				ShellExecute(NULL, L"open", strUrl, NULL, NULL, SW_SHOW);
			}

			/*
			Tough Disabled:
			if(!OsIsXpOrEarlier())
				LaunchDefaultBrowser(strUrl);
			else
				ShellExecute(NULL,_T("open"),strUrl,NULL,NULL,SW_NORMAL);
			*/
			//-----------------------------------------------------------------------
			
			USES_CONVERSION;
			std::string base64 = W2A(strUrl);
			base64 = dsCommonn::base64_encode(base64);
			
			CString str(SearchEngineMgr::getInstance()->m_strSearchStatFormat + base64.c_str());

			dsCloud_SendDataCount(str, FALSE, TRUE);
		

			//RequestMgr* pStart = new RequestMgr((LPWSTR)(LPCTSTR)str);
			//if( pStart )
			//	pStart->StartRequest();

			/*if(!ShellExecute(NULL,_T("open"),strUrl,NULL,NULL,SW_SHOWDEFAULT))
			{
				ShellExecute(NULL,_T("open"),_szIE,strUrl,NULL,SW_SHOWDEFAULT);
				
			}*/

			//----------------------------------------------------------------
			//Tough Disabled 		
			//ListView_UrlMatch::getInstance()->Show(FALSE);					
			//SetMsgHandled(TRUE);
			
			//----------------------------------------------------------------

			::PostMessage(GetParent(), MG_MSG_SHRINK, 0, 2);
			SetMsgHandled(TRUE);		

			
		}
		else if(VK_UP == key){
			CString str;
			if(ListView_UrlMatch::getInstance()->SelectPrevMatchedItem(str)){

				m_bIgnoreChange = TRUE;

				SetMsgHandled(TRUE);

				SetWindowText(str);

				SetSel(-1, -1);
			}
		}
		else if(VK_DOWN == key){
			CString str;
			if(ListView_UrlMatch::getInstance()->SelectNextMatchedItem(str)){
				m_bIgnoreChange = TRUE;

				SetMsgHandled(TRUE);

				SetWindowText(str);

				SetSel(-1, -1);
			}
		}
		else if (VK_LEFT == key || VK_RIGHT == key)
		{
			// SetTimer(1002, 1000, NULL);
			/*
			LONG nStart, nEnd;
			GetSel(nStart, nEnd);
			if (nStart == nEnd && nStart >= 1)
			{
				SetSel(nStart, nStart);
			}
			*/						
			Invalidate(FALSE);
			return;

		}
		else if(VK_BACK == key)
		{
			m_bIgnoreUpdate = FALSE;

			if(0 == GetTextLength())
			{
				SetMsgHandled(TRUE);
			}

			m_bBack = TRUE;

			LONG nStart,nEnd;
			GetSel(nStart, nEnd);

			if(nEnd > nStart){

			}
	
		}
		else if(VK_ESCAPE == key){
	
			ConfigMgr::getInstance()->HidePopopWnd();

		}

	}
	
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags){

		//-------------------------------------------
		//2015 3.5 Tough Added:
		SetMsgHandled(FALSE);
		return;

		//-------------------------------------------

		long nStart,nEnd;

		GetSel(nStart, nEnd);

		if(nEnd > nStart){

			CString strEdit = GetEditText();

			if (nChar == strEdit.GetAt(nStart)){

				LockWindowUpdate();

				SetSel(++nStart, nEnd);

				LockWindowUpdate(FALSE);
				ListView_UrlMatch::getInstance()->UpdateMatchedContent(strEdit.Left(nStart), FALSE, TRUE);

				SetIgnore();
				return;
			}
		}		

		SetMsgHandled(FALSE);
	}

	void OnLButtonDown(UINT keys, const CPoint& point)
	{	
		SetMsgHandled(FALSE);

		static DWORD dw = 0;
		DWORD dwNew = GetTickCount();
		if (dwNew - dw < 1000)
		{
			ATLTRACE(_T("dwNew - dw < 1000"));			
			return;
		}

		dw = dwNew;

		/*
		if (GetTextLength() == 0)
		{
			::PostMessage(GetParent(), MG_MSG_DISADAPTERDLG, 0, 0);
		}
		else
		{
			//CString strEdit = GetEditText();
			//LockWindowUpdate(FALSE);
			//ListView_UrlMatch::getInstance()->UpdateMatchedContent(strEdit, FALSE, TRUE, TRUE);

			//SetIgnore();
		}
		*/

		//if (GetFocus() != m_hWnd)
		
		SetTimer(1001, 100, NULL);
				
	}

	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/){

		DefWindowProc(WM_LBUTTONDOWN, wParam, lParam);

		return 0;

	}

	void OnRButtonDown(UINT keys, const CPoint& point)
	{
		//if (GetFocus() != m_hWnd)
		//{
		//	SetFocus(); 
		//	SetSelAll();
		//}

		SetMsgHandled(FALSE);

		
	}

	void OnRButtonUp(UINT nFlags, CPoint point){

		::PostMessage(GetParent(), MG_MSG_DISADAPTERDLG, 0, 2);	

		HMENU hMenu = CreatePopupMenu();
		GETTEXTLENGTHEX stuGlt;
		stuGlt.flags = GTL_NUMBYTES;
		stuGlt.codepage = CP_ACP;
		int nResult = SendMessage(m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM)&stuGlt, 0);
		if ( !nResult)
		{
			AppendMenu(hMenu, MF_STRING|MF_DISABLED|MF_GRAYED, 1, _T("剪切(&T)"));
			AppendMenu(hMenu, MF_STRING|MF_DISABLED|MF_GRAYED, 2, _T("复制(&C)"));
		}
		else
		{
			AppendMenu(hMenu, MF_STRING, 1, _T("剪切(&T)"));			
			AppendMenu(hMenu, MF_STRING, 2, _T("复制(&C)"));
		}
		AppendMenu(hMenu, MF_STRING, 3, _T("粘贴(&P)"));
		AppendMenu(hMenu, MF_SEPARATOR, 4, _T(""));
		AppendMenu(hMenu, MF_STRING, 5, _T("收缩魔框(&S)"));  //缩回图标	 
		//AppendMenu(hMenu, MF_STRING, 6, _T("关于魔框(&A)"));
		//AppendMenu(hMenu, MF_STRING, 7, _T("反馈建议(&F)"));
		AppendMenu(hMenu, MF_STRING, 8, _T("退出(&E)"));



		//*lphmenu = hMenu;

		POINT pt;
		GetCursorPos(&pt);
		int nRet = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_TOPALIGN|TPM_RETURNCMD, 
			pt.x, pt.y, 0, GetParent(), NULL);

		switch(nRet)
		{
		case 1:
			::SendMessage(m_hWnd, WM_CUT, 0, 0);
			break;
		case 2:
			::SendMessage(m_hWnd, WM_COPY, 0, 0);
			break;

		case 3:
			::SendMessage(m_hWnd, WM_PASTE, 0, 0);
			Invalidate(FALSE);
			break;

		case 5:
			g_bShrink_Switch = TRUE;			
			::SendMessage(GetParent(), MG_MSG_SHRINK, 0, 0);
			break;

		case 6:
			{
				//::SendMessage(m_hEditWnd, WM_PASTE, 0, 0L);
				//
				//TCHAR sz[MAX_PATH];
				//::GetWindowText(m_hEditWnd, sz, MAX_PATH);				
				// //SendMessage(::GetParent(m_hEditWnd), WM_NAVIGATE, 0, (LPARAM)sz);

				::SendMessage(GetParent(), WM_COMMAND, IDC_BTN_ABOUT, 0);

			}
			break;
		case 7:
			{				
				CString strUrl(L"http://so.languang.com/feedback.htm");				
				int nRet = OpenUrlByDefaultBrowser(strUrl);
				if (nRet == 0)
				{
					ShellExecute(NULL, L"open", strUrl, NULL, NULL, SW_SHOW);
				}

			}
			break;
		case 8:
			{
				::SendMessage(GetParent(), MG_MSG_EXIT, 0, 0);
			}
			break;
		}		

		::PostMessage(GetParent(), WM_CANCELMODE, 0, 0);		

	}
	
	BOOL OnEraseBkgnd(CDCHandle dc)
	{		
		//ATLTRACE(_T("dsTRansEdit::OnEraseBkgnd"));		
				
		//DoubleBufferPaint dbuffPaint(m_hWnd);

		//m_ptrVisualStyle->DrawBackground(m_hWnd, dbuffPaint.GetDC(), dbuffPaint.GetPaintRect());

		//::SetBkMode(dbuffPaint.GetDC()/*hdc*/, TRANSPARENT);

		//m_pTestFrame->renderHDC(dbuffPaint.GetDC());

		//CRect rcInvalideCopyWndToBitmap(GetParent(), m_hBitmap);			
	
		//RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);
				

		return 1;
	}		

	BOOL SetRangeColor(IRichEditOle* pOle, int nStart, int nEnd, COLORREF col)
	{	
		if (!pOle)
			return 0;

/*		CComPtr<ITextDocument> pTextDoc;
		HRESULT hr = pOle->QueryInterface(__uuidof(ITextDocument),(void**)&pTextDoc);
		if (SUCCEEDED(hr))
		{
			CComPtr<ITextRange> pRange;
			hr = pTextDoc->Range(nStart, nEnd, &pRange);
			if (SUCCEEDED(hr))
			{
				CComPtr<ITextFont> font;
				pRange->GetFont(&font);
				font->SetForeColor(col);
			}
		}	*/	

		return TRUE;
	}	

	void OnSetFocus(HWND hOldWnd)
	{	
		//::SendMessage(GetParent(), MG_MSG_SETHOTBKGND, 0, 0);

		//UpdateColor(TRUE);

		//static BOOL _bFirstTime = TRUE;

		//if(_bFirstTime)
		//{

		//	//TCHAR szText[20];
		//	//GetWindowText(szText, 20);
		//	_bFirstTime = FALSE;
		//	SetWindowText(L"");
		//}
		RegisterHotKey(this->m_hWnd, MG_MSG_COPY, MOD_CONTROL, 'C');
		RegisterHotKey(this->m_hWnd, MG_MSG_PASTE, MOD_CONTROL, 'V');
		RegisterHotKey(this->m_hWnd, MG_MSG_CUT, MOD_CONTROL, 'X');
		SetMsgHandled(FALSE);

		/*if(m_bLBtnDown && 0 == GetWindowTextLength()){
			m_bLBtnDown = FALSE;		
			ListView_HotNews::getInstance()->Show(TRUE);
		}*/
		//		

		//TCHAR szText[MAX_PATH];
		//GetWindowText(szText, MAX_PATH);
		LONG nStart, nEnd;
		GetSel(nStart, nEnd);

		if (nStart == nEnd && nStart > 0)
		{
			TCHAR sz[MAX_PATH];			
			GetWindowText(sz, MAX_PATH);

			CString strText(sz);
			strText.TrimLeft();
			strText.TrimRight();

			if (strText.GetLength() == 0)
			{
				ATLTRACE(_T("OnSetFocus, SetSel(0, 0)"));
				//SetSel(0, 0);
				//SetTimer(1003, 300, NULL);
				 SetWindowText(_T(""));
			}

		}		

	}

	void OnKillFocus(HWND hFocus)
	{	
		//-----------------------------------------------------------------
		//Tough Added:							
		if (!ConfigMgr::getInstance()->IsWndFocusKing(hFocus))
		{			
			TCHAR sz[MAX_PATH];
			GetClassName(GetForegroundWindow(), sz, MAX_PATH);		
			ATLTRACE(_T("OnKillFocus:%s"), sz);
			if (_tcscmp(sz, _T("MagicHtmlWnd")) != 0 && 
				_tcscmp(sz, _T("MagicBoxFrame")) != 0 &&
				_tcscmp(sz, _T("Internet Explorer_Server")) != 0)
			{				
				::PostMessage(GetParent(), MG_MSG_DISADAPTERDLG, 0, 2);					
			}
							
		}		

		m_bIgnoreUpdate = FALSE;
		Invalidate(FALSE);
		
		//-----------------------------------------------------------------
		
		//UpdateColor(FALSE);
		UnregisterHotKey(this->m_hWnd, MG_MSG_COPY);
		UnregisterHotKey(this->m_hWnd, MG_MSG_PASTE);
		UnregisterHotKey(this->m_hWnd, MG_MSG_CUT);
		SetMsgHandled(FALSE);	
		
	} 

	BOOL OnEditChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	
		/*
		TCHAR szText[MAX_PATH];

		GetWindowText(szText, MAX_PATH);

		
		VEC_UA vecUrl;
		VEC_UA vecWord;

		UrlAdapterMgr::getInstance()->Macth(szText, vecUrl, vecWord);
		*/

		return TRUE;
	}

	void OnMouseMove(UINT nFlags, CPoint point){
		
		::SendMessage(GetParent(), MG_MSG_SETHOTBKGND, 0, 0);

		SetMsgHandled(FALSE);

	}

public:
	
	void Update()
	{
		SetFocus();
		TCHAR sz[MAX_PATH];
		GetWindowText(sz, MAX_PATH);
				
		SetWindowText(sz);		
		int nLen = _tcslen(sz);
		SetSel(nLen, nLen);

	}

	//void LaunchDefaultBrowser(LPCTSTR lpszUrl)  
	//{  
	//	HKEY hDefBrowserPos = NULL;  
	//	wstring wstrDefBrowserPath = L"iexplore.exe";  
	//	WCHAR wszBuffer[MAX_PATH + 1] = {0};  
	//	DWORD dwDataSize = sizeof(wszBuffer);  
	//	HKEY hkey;
	//	if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\Ftp\\UserChoice\\", &hkey)) 
	//	{  
	//		DWORD dwType = REG_SZ;
	//		if( ERROR_SUCCESS == RegQueryValueEx( hkey, L"Progid", NULL, &dwType, (LPBYTE)wszBuffer, &dwDataSize))
	//		{
	//			wstring wstrDefBrowserPos = wszBuffer;  
	//			wstrDefBrowserPos += L"\\shell\\open\\command\\";  
	//			dwDataSize = sizeof(wszBuffer);  
	//			HKEY hkey2;
	//			if( ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, wstrDefBrowserPos.c_str(), &hkey2) && !RegQueryValueEx( hkey2, NULL, NULL, &dwType, (LPBYTE)wszBuffer, &dwDataSize)) 
	//			{  
	//				// 解出exe 路径.  
	//				wstrDefBrowserPath = wszBuffer;  
	//				wstring::size_type leftQuotation = wstrDefBrowserPath.find(L'"');  
	//				if (leftQuotation != wstring::npos)  
	//				{  
	//					wstring::size_type rightQuotation = wstrDefBrowserPath.find(L'"', leftQuotation + 1);  
	//					if (rightQuotation != wstring::npos)  
	//					{  
	//						wstrDefBrowserPath.assign(  
	//							wstrDefBrowserPath.begin() + leftQuotation + 1,  
	//							wstrDefBrowserPath.begin() + rightQuotation  
	//							);  
	//					}  
	//				}  
	//			}  
	//			RegCloseKey(hkey2);
	//		}
	//	
	//	}  
	//	RegCloseKey(hkey);
	//	::ShellExecuteW( NULL, L"open", wstrDefBrowserPath.c_str(), lpszUrl, NULL, SW_NORMAL);  
	//}  

	CString GetDefaultExplorer(){
		HKEY hkRoot,hSubKey; //定义注册表根关键字及子关键字
		char ValueName[256];
		unsigned char DataValue[256];
		unsigned long cbValueName=256;
		unsigned long cbDataValue=256;
		char ShellChar[256]; //定义命令行
		DWORD dwType;

		DWORD dwError = 0;

		//打开注册表根关键字
		//if(RegOpenKeyA(HKEY_CLASSES_ROOT,NULL,&hkRoot)==ERROR_SUCCESS)
		{
			//打开子关键字
			if(RegOpenKeyExA(NULL,
				"HKEY_CLASSES_ROOT\\htmlfile\\shell\\open\\command",
				0,
				KEY_QUERY_VALUE,
				&hSubKey)==ERROR_SUCCESS)
			{
				//读取注册表，获取默认浏览器的命令行     
				RegEnumValueA(hSubKey, 
					0,
					ValueName,
					&cbValueName,
					NULL,
					&dwType,
					DataValue,
					&cbDataValue);
				// 调用参数（主页地址）赋值
				strcpy(ShellChar,(char *)DataValue);
				strcat(ShellChar," www.neaase.net/~xiaohui");
				// 启动浏览器
				WinExec(ShellChar,SW_SHOW);

			}
			else
				dwError = GetLastError();
		}
		//else
			dwError = GetLastError();
		//关闭注册表
		RegCloseKey(hSubKey);
		RegCloseKey(hkRoot);

		return L"";
	}

	inline BOOL IgnoreChange(){
		
		BOOL bOld = m_bIgnoreChange;
		if(m_bIgnoreChange)
			m_bIgnoreChange = !m_bIgnoreChange;
		return bOld;
	}

	inline void SetIgnore(BOOL bIgnore = TRUE){
		m_bIgnoreChange = bIgnore;
	}

	inline BOOL DidUserTypeBack(){
		BOOL bOld = m_bBack;
		if(m_bBack)
			m_bBack = !m_bBack;
		return bOld;
	}

	CString GetEditText()
	{
		CString strText;

		int nLen = GetTextLength();
		if (nLen > 0)
		{
			TCHAR* pText = new TCHAR[nLen + 1];
			GetWindowText(pText, nLen + 1);
			strText = pText;
			delete pText; 
		}

		return strText;
	}

	//void SetEditText(LPCTSTR lpszText)
	//{
	//	int nLen =  GetTextLength();
	//	TCHAR *pszText = new TCHAR[nLen + 1];	
	//	GetWindowText(pszText, nLen + 1);		

	//	if (lstrcmp(lpszText, pszText))
	//	{
	//		SetWindowText(lpszText);			
	//		//UpdateColor();
	//		//Invalidate(FALSE);
	//		
	//	}
	//	delete [] pszText;
	//}

	BOOL UpdateColor(BOOL bNormal = FALSE)
	{
		IRichEditOle* pOle = GetOleInterface();
		if (!pOle)
			return 0;

		CString strUrl = GetEditText();			
		if (bNormal)
		{
			SetRangeColor(pOle, 0, strUrl.GetLength(), RGB(0, 0, 0));
			return 1;
		}

		if (strUrl.Find(L"http://") != -1)
		{
		    int nStart = strUrl.Find(L"www.", 7) != -1 ? 11 
													   : strUrl.Find('.', 7) + 1;
			
			SetRangeColor(pOle, 0, nStart, RGB(172, 168, 153));			
			int nOrg = nStart;
			nStart = strUrl.Find(L"/", nStart);					
			if (nStart != -1)
			{
				SetRangeColor(pOle, nStart, strUrl.GetLength(), RGB(172, 168, 153));
				SetRangeColor(pOle, nOrg, nStart, RGB(0, 0, 0));
			}
			else
			{
				SetRangeColor(pOle, nOrg, strUrl.GetLength(), RGB(0, 0, 0));
			}

		}		

		return 1;
	}

	void Init(HWND hParentWnd, HFONT hFont, COLORREF cr)
	{
		LoadLibrary(L"riched20.dll");
		Create(hParentWnd, CRect(0, 0, 0, 0), L"", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|ES_LEFT|ES_AUTOHSCROLL);		
		

		SendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
		SendMessage(EM_SETTEXTMODE, TM_PLAINTEXT | TM_SINGLELEVELUNDO);

		m_OleCallback.SetEditWnd(m_hWnd);
		SetOleCallback(&m_OleCallback);		

		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, 0); lfreg = lfIcon;
		lfIcon.lfHeight = -14;
		lfIcon.lfWidth = 0;
		lfIcon.lfEscapement = 0;
		lfIcon.lfOrientation = 0;
		lfIcon.lfWeight =  FW_NORMAL;  //FF_SCRIPT; //
		lfIcon.lfItalic = 0;
		lfIcon.lfUnderline = 0;
		lfIcon.lfStrikeOut = 0;		
		lfIcon.lfCharSet = ANSI_CHARSET;
		lfIcon.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lfIcon.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfIcon.lfQuality = PROOF_QUALITY;
		lfIcon.lfPitchAndFamily = VARIABLE_PITCH;	
		if  (wcsstr(lfIcon.lfFaceName, _T("YaHei UI")))
		{
			_tcscpy(lfIcon.lfFaceName, _T("微软雅黑"));
		}

		CString strDebug;
		strDebug.Format(_T("FaceName:%s"), lfIcon.lfFaceName);
		dsCommonn::WriteLog(strDebug);
		
		//_tcscpy(lfIcon.lfFaceName, _T("微软雅黑")); CWindowDC dc(m_hWnd);
		m_hRegularFont = ::CreateFontIndirect(&lfIcon);
		SetFont(m_hRegularFont);	

		CWindowDC dc(m_hWnd);
		HFONT hOldFont = dc.SelectFont(m_hRegularFont);
		dc.GetTextExtent(_T(" "), 1, &m_TextMetrix);
		dc.SelectFont(hOldFont);
		
		ModifyStyleEx(0, WS_EX_TRANSPARENT);
	
		CHARFORMAT2 cf;
		ZeroMemory(&cf, sizeof(CHARFORMAT2));
		cf.cbSize = sizeof(CHARFORMAT2);

		//LOGFONT lf;
		if(NULL != hFont)
			::GetObject((HANDLE)hFont, sizeof LOGFONT, &lfIcon);
		
		Logfont2CharFormat(cf, lfIcon, cr);
		cf.dwMask = CFM_COLOR;   //| CFM_BACKCOLOR   
		//cf.crBackColor = RGB(255, 0, 0);
		
		//cf.dwEffects = CFE_AUTOBACKCOLOR;


		//cf.cbSize = sizeof(CHARFORMAT);
		//cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE |
		//	CFM_ITALIC | CFM_SIZE;
		///*cf.dwEffects = CFE_BOLD|
		//	CFE_ITALIC ;
		//	cf.yHeight = 12  * 20;*/
		cf.crTextColor = RGB(255, 255, 255);
		///*strcpy*/wcscpy(cf.szFaceName, L"宋体");5
		//cf.bCharSet = 0;
		//cf.bPitchAndFamily = 0;

		SetDefaultCharFormat(cf);				
		
	}

	void SetFontAttribute(HFONT hFont, COLORREF cr){
		
		CHARFORMAT cf;
		ZeroMemory(&cf, sizeof(CHARFORMAT));
		cf.cbSize = sizeof(CHARFORMAT);

		//LOGFONT lf;
		if(NULL != hFont)
			::GetObject((HANDLE)hFont, sizeof LOGFONT, &lfIcon);

		Logfont2CharFormat(cf, lfIcon, cr);
		//cf.cbSize = sizeof(CHARFORMAT);
		//cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE |
		//	CFM_ITALIC | CFM_SIZE;
		///*cf.dwEffects = CFE_BOLD|
		//	CFE_ITALIC ;
		//	cf.yHeight = 12  * 20;*/
		//cf.crTextColor = RGB(255, 255, 255);
		///*strcpy*/wcscpy(cf.szFaceName, L"宋体");5
		//cf.bCharSet = 0;
		//cf.bPitchAndFamily = 0;

		SetDefaultCharFormat(cf);
		
	}


	BOOL ResetFocus()
	{
		::SetFocus(GetParent());		
		SetSelNone();		
		
		return 1;
	}

	void Logfont2CharFormat(CHARFORMAT &cf, const LOGFONT lf, const COLORREF textcolor)
	{
		//CFE_AUTOCOLOR
		cf.dwMask = CFM_BOLD|CFM_CHARSET|CFM_COLOR|CFM_FACE|CFM_ITALIC|
			CFM_SIZE|CFM_UNDERLINE|CFM_STRIKEOUT; 

		BOOL bIsItalic = lf.lfItalic ;
		BOOL bIsBold = (lf.lfWeight >= FW_BOLD);//? FW_NORMAL:FW_BOLD;
		BOOL bIsUnderline = lf.lfUnderline ;
		BOOL bIsStrickout = lf.lfStrikeOut ;

		cf.dwEffects |= CFE_PROTECTED;
		cf.dwEffects &= ~CFE_AUTOCOLOR;
		if (bIsBold) 
		{ 
			cf.dwEffects |= CFE_BOLD; 
		} 
		else
		{
			cf.dwEffects &= ~CFE_BOLD; 
		}

		if (bIsItalic) 
		{ 
			cf.dwEffects |= CFE_ITALIC; 
		} 
		else
		{
			cf.dwEffects &= ~CFE_ITALIC; 
		}

		if (bIsUnderline) 
		{ 
			cf.dwEffects |= CFE_UNDERLINE; 
		} 
		else
		{
			cf.dwEffects &= ~CFE_UNDERLINE; 
		}

		if (bIsStrickout) 
		{ 
			cf.dwEffects |= CFM_STRIKEOUT; 
		} 
		else
		{
			cf.dwEffects &= ~CFM_STRIKEOUT; 
		}

		//设置属性
		cf.yHeight =  -15 * lf.lfHeight;
		cf.crTextColor = textcolor;//;//m_Font_Color;
		cf.bCharSet = lf.lfCharSet ;
		cf.bPitchAndFamily = lf.lfPitchAndFamily ;
		_stprintf(cf.szFaceName, lf.lfFaceName);
	}

	BOOL CopyWndToBitmap(HWND hWnd, HBITMAP& hBitmap)
	{
		CRect rt;
		::GetClientRect(hWnd, &rt);

		if (rt.IsRectEmpty())
			return NULL;

		if (hBitmap)
		{
			::DeleteObject(hBitmap);
			hBitmap = NULL;
		}

		CRect rtWnd;
		::GetWindowRect(hWnd, &rtWnd);

		if (rtWnd.left < 0 || rtWnd.top < 0)
			return FALSE;

		if (rtWnd.bottom > GetSystemMetrics(SM_CYSCREEN) || rtWnd.right > GetSystemMetrics(SM_CXSCREEN))
			return FALSE;

		rt.InflateRect(0, 0, 0, 0);	  	

		HDC hScrDC,hMemDC;               

		hScrDC = ::GetDC(hWnd);
		hMemDC = ::CreateCompatibleDC(hScrDC);   

		hBitmap = ::CreateCompatibleBitmap(hScrDC, rt.Width(), rt.Height());   
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);   
		//::FillRect(hMemDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		::BitBlt(hMemDC, 0, 0, rt.Width(), rt.Height(), hScrDC, 0, 0, SRCCOPY);   

		hBitmap = (HBITMAP)::SelectObject(hMemDC, hOldBitmap);   	
		DeleteDC(hScrDC);   
		DeleteDC(hMemDC);   

		return TRUE;
	}

	BOOL CopyWndToBitmap(HWND hWnd)
	{
		CRect rt;
		::GetClientRect(hWnd, &rt);

		if (rt.IsRectEmpty())
			return NULL;

		if (m_hBitmap)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}

		CRect rtWnd;
		::GetWindowRect(hWnd, &rtWnd);

		if (rtWnd.left < 0 || rtWnd.top < 0)
			return FALSE;

		if (rtWnd.bottom > GetSystemMetrics(SM_CYSCREEN) || rtWnd.right > GetSystemMetrics(SM_CXSCREEN))
			return FALSE;

		rt.InflateRect(0, 0, 0, 0);	  	

		HDC hScrDC,hMemDC;               

		hScrDC = ::GetDC(hWnd);
		hMemDC = ::CreateCompatibleDC(hScrDC);   

		m_hBitmap = ::CreateCompatibleBitmap(hScrDC, rt.Width(), rt.Height());   
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);   
		::FillRect(hMemDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		::BitBlt(hMemDC, 0, 0, rt.Width(), rt.Height(), hScrDC, 0, 0, SRCCOPY);   

		m_hBitmap = (HBITMAP)::SelectObject(hMemDC, hOldBitmap);   	
		DeleteDC(hScrDC);   
		DeleteDC(hMemDC);   

		return TRUE;
	}



private:
	int  m_nOldLen;	
	IRichEditOleCallbackEx m_OleCallback;

	LOGFONT lfIcon,lfreg;
	HFONT   m_hRegularFont;	

	BOOL m_bIgnoreChange;
	BOOL m_bIgnoreUpdate;

	BOOL m_bLBtnDown;
	BOOL m_bBack;
	HBITMAP m_hBitmap;
	SIZE    m_TextMetrix;
		

public:
	HWND                m_hListBox;
};


#endif /*__DSTRANSEDIT_H__*/