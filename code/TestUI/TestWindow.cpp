#include "StdAfx.h"
#include "TestWindow.h"
#include <commctrl.h>
#include "..\JCfsClient\JCfsClient_i.h"
CAppModule Module;



enum ListColumn
{
	ListColumn_Index = 0,

	ListColumn_FileName,				   // 文件名称
	ListColumn_FilePath,				   // URL

	ListColumn_Leave,
	LC_Null = 999,
};

struct ListColumnDef
{
	ListColumn		listColumn;
	LPCTSTR			strText;
	INT				nWidth;
};

static ListColumnDef g_ListColumnDef[] =
{
	{ListColumn_Index		,L"序号"		,	70	},
	{ListColumn_FileName	,L"文件名称"	,	200	},
	{ListColumn_FilePath	,L"文件路径"	,	500	},
	{ListColumn_Leave		,L"级别"		,	60	},
	{LC_Null				,NULL			,	0	},
};



CTestWindow::CTestWindow(void)
{
}


CTestWindow::~CTestWindow(void)
{
}

LRESULT CTestWindow::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	DlgResize_Init();

	m_strextension =  _T("*.*");
	CMessageLoop *pLoop = Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	UIAddMenuBar(m_hWnd);

	

	this->CenterWindow();


	m_dlTaskMgrListControl = (GetDlgItem(IDC_DL_TASK_MGR_LIST));
	m_dlTaskMgrListControl.ModifyStyle(LVS_TYPEMASK,	LVS_REPORT | LVS_SINGLESEL |LVS_OWNERDATA);

	m_dlTaskMgrListControl.SetExtendedListViewStyle(
		LVS_EX_GRIDLINES |
		LVS_EX_FULLROWSELECT |
		LVS_EX_FLATSB |
		LVS_EX_TWOCLICKACTIVATE |
		LVS_EX_ONECLICKACTIVATE |
		LVS_EX_TRACKSELECT |
		LVS_EX_GRIDLINES|
		LVS_EX_DOUBLEBUFFER);

	 DWORD dwStyle = m_dlTaskMgrListControl.GetExtendedListViewStyle();

	 dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	 dwStyle |= LVS_EX_GRIDLINES;    //网格线（只适用与report风格的listctrl）

	 m_dlTaskMgrListControl.SetExtendedListViewStyle(dwStyle); //设置扩展风格
	 for (int nIndex = 0 ; g_ListColumnDef[nIndex].strText != NULL ; nIndex++)
	 {

		 LV_COLUMN lvCol = {0};
		 lvCol.pszText = (LPWSTR)g_ListColumnDef[nIndex].strText;
		 lvCol.cx = g_ListColumnDef[nIndex].nWidth;
		 m_dlTaskMgrListControl.InsertColumn(g_ListColumnDef[nIndex].listColumn, g_ListColumnDef[nIndex].strText, LVCFMT_LEFT , g_ListColumnDef[nIndex].nWidth);
	 }
	return 0;
}


LRESULT CTestWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	CMessageLoop *pLoop = Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	//pLoop->RemoveIdleHandler(this);
	//pLoop->RemoveMessageFilter(this);


	


	return 0;
}

LRESULT CTestWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{

	UTIL::com_ptr<IExit> pExit;
	m_pRot->GetObject(CLSID_MsEnv, re_uuidof(IExit), (void**)&pExit);
	if ( pExit )
	{
		pExit->NotifyExit();
	}


    DestroyWindow();
    return 0;
}

BOOL CTestWindow::PreTranslateMessage(MSG *pMsg)
{
	return CWindow::IsDialogMessage(pMsg);;
}


BOOL CTestWindow::OnIdle()
{
	UIUpdateMenuBar(FALSE, TRUE);
	return FALSE;
}


LRESULT CTestWindow::OnCommand(UINT , WPARAM wParam, LPARAM , BOOL &bHandled)
{
	WORD crtid = LOWORD(wParam);
	WORD code  = HIWORD(wParam);

	return 0;
}


STDMETHODIMP CTestWindow::Init( void* )
{

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);
	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	//CBVDownloadWindow *bvWnd = new CBVDownloadWindow();
	Module.Init(NULL , GetModuleInstance());


	return S_OK;
}


STDMETHODIMP CTestWindow::Uninit()
{
	SAFE_RELEASE(m_pRot);



	Module.Term();
	return S_OK;
}

STDMETHODIMP CTestWindow::Start()
{

	CreateInstance(CLSID_MsSafeDwordVector, NULL, re_uuidof(IMsDwordVector), (void**)&m_pResult);
	CMessageLoop theLoop;
	Module.AddMessageLoop(&theLoop);

	if (Create(NULL))
	{
		ShowWindow(SW_SHOW);
		//theLoop.Run();
	}

	return S_OK;
}


STDMETHODIMP CTestWindow::Stop()
{
	return S_OK;
}

LRESULT CTestWindow::OnTimer( UINT , WPARAM wParam, LPARAM , BOOL& bHandled )
{
	DWORD wTimerID = wParam;

	return 0;
}


LRESULT CTestWindow::OnExecute(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/)
{
	auto func = (std::function<void()> *)lParam;
	(*func)();
	delete func;
	return 0;
}

LRESULT CTestWindow::OnResultListGetDispinfo(INT n,LPNMHDR lpNmhdr, BOOL& bHandle)
{
	LV_ITEM *pItem=&(((NMLVDISPINFO*)lpNmhdr)->item);

	if (pItem->iItem != m_dwIndex)
	{
		ZeroMemory(&m_Result, sizeof(m_Result));
		m_dwIndex = pItem->iItem;
		HRESULT hr = m_pDiskSearch->GetResultAt(m_dwIndex, 555, m_Result);
	}

	if (pItem->mask & LVIF_TEXT)
	{
		switch(pItem->iSubItem)
		{
		case ListColumn_FileName:
			{
				if(pItem->mask&LVIF_IMAGE)
				{
					//pItem->iImage= m_SearchCollectInfo.IcoIndex;
				}

				if (wcslen(m_Result.szName) == 2 && m_Result.szName[1] == ':'){
					DWORD dwMaxComLen = 0,dwFileSysFlag = 0;
					TCHAR szVolumeName[MAX_PATH] = {0};
					TCHAR fileSysBuf[MAX_PATH] = {0};

					CString strDriverName;
					strDriverName.Format(_T("%s\\"), m_Result.szName);
					UINT driveType = GetDriveType(strDriverName);
					if (!GetVolumeInformation(strDriverName,szVolumeName,MAX_PATH * 2,NULL,&dwMaxComLen,&dwFileSysFlag,fileSysBuf,MAX_PATH))
					{
						break;
					}


					CString strFormat = _tcslen(szVolumeName) ? szVolumeName : (driveType ==DRIVE_FIXED ?_T("本地磁盘") : _T("移动磁盘"));
					strFormat.AppendFormat(_T("(%s)"), m_Result.szName);
					_tcscpy(pItem->pszText, strFormat);
				}
				else{
					_tcscpy(pItem->pszText, m_Result.szName);
				}

				break;
			}
		case ListColumn_FilePath:
			{
				//pItem->iImage= m_Result.IcoIndex;
				_tcscpy(pItem->pszText, m_Result.szPath);
				break;
			}
		}
	}

	return 0;
}

void CTestWindow::Post(const std::function<void()> &f)
{
	auto func = new std::function<void()>(f);
	PostMessage( WM_EXECUTE_MSG, 0, (LPARAM)func);
}


BOOL CTestWindow::GetSearchExtension(CString& strText)
{

	CString StrText, StrPath;
	GetDlgItemText(IDC_EDIT_NAME, StrText);
	GetDlgItemText(IDC_EDIT_PATH, StrPath);
	CString strSearch = StrText;

	INT nPos = strSearch.Find('.');
	if (nPos != -1)
	{
		strSearch = strSearch.Mid(nPos);
	}
	else
	{
		strSearch.Empty();
	}

	strSearch.Trim();
	strSearch.Trim('.');
	if (strSearch.GetLength())
	{
		strText = strSearch;
	}

	return strText.GetLength() ? TRUE : FALSE;
}


BOOL CTestWindow::GetSearchText(CString& strText)
{
	CString StrText, StrPath;
	GetDlgItemText(IDC_EDIT_NAME, StrText);
	GetDlgItemText(IDC_EDIT_PATH, StrPath);

	CString strSearch = StrText;

	INT nPos = strSearch.Find('.');
	if (nPos != -1)
	{
		strSearch = strSearch.Left(nPos);
	}

	strSearch.Trim();
	if (strSearch.GetLength())
	{
		strText = strSearch;
	}

	return TRUE;
}

void CTestWindow::Query()
{
	CString StrText, StrPath;
	GetDlgItemText(IDC_EDIT_NAME, StrText);
	GetDlgItemText(IDC_EDIT_PATH, StrPath);

	BOOL bFullWord = CButton(GetDlgItem(IDC_CHECK_FULLWORD)).GetCheck();
	BOOL bFullCase = CButton(GetDlgItem(IDC_CHECK_CASE)).GetCheck();
	BOOL bSubPath = CButton(GetDlgItem(IDC_CHECK_SUBPATH)).GetCheck();


	QueryCondition queryCondition;
	queryCondition.dwConditionMask = 0; 

	CString strExten;

	m_strextension =  _T("*.*");
	if (GetSearchExtension(strExten))
	{
		queryCondition.lpstrExtension = strExten;
		queryCondition.dwConditionMask |= QueryConditionMask_File; //有扩展名，只查询文件
	}
	else
	{
		if (m_strextension == _T("*.*")) //查询所有
		{
			queryCondition.dwConditionMask |= QueryConditionMask_File; 
			queryCondition.dwConditionMask |= QueryConditionMask_Dir; 
		}
		else if(m_strextension == _T("*")) //只查询目录
		{
			queryCondition.dwConditionMask |= QueryConditionMask_Dir; 
		}
		else if (m_strextension.GetLength() == 0)
		{
			queryCondition.dwConditionMask |= QueryConditionMask_File | QueryConditionMask_Dir; 
		}
		else{
			queryCondition.dwConditionMask |= QueryConditionMask_File; 
			//queryCondition.lpstrExtension = m_strextension;
		}
	}




	if (wcslen(queryCondition.lpstrIncludeDir))
	{
		if (bSubPath)
		{
			queryCondition.dwConditionMask |= QueryConditionMask_Sub;
		}

	}
	if ( bFullCase )
	{
		queryCondition.dwConditionMask |= QueryConditionMask_Case;
	}

	if ( bFullWord )
	{
		queryCondition.dwConditionMask |= QueryConditionMask_Full;
	}
	if( bSubPath )
	{
		queryCondition.dwConditionMask |= QueryConditionMask_Sub;
	}

	CString strName;
	GetSearchText(strName);

	CString strPath= StrPath; 
	if(strPath.GetLength() && !strName.GetLength() && !_tcslen(queryCondition.lpstrExtension))
	{
		strName = _T("*");
	}

	if (!strName.GetLength() && !_tcslen(queryCondition.lpstrExtension) && !_tcslen(queryCondition.lpstrIncludeDir))
	{
		strName = _T(":");
	}
	queryCondition.lpstrFileName = strName;
	queryCondition.lpstrIncludeDir = strPath;

	DWORD dwCount = 0;
	if ( m_pDiskSearch)
	{
		m_pResult->Clear();
		m_pDiskSearch->Query(queryCondition);
		
	}
	

}

STDMETHODIMP CTestWindow::NotifyExit( bool* bExit /*= NULL*/ )
{
	return S_OK;
}


STDMETHODIMP CTestWindow::OnDiskSearch_FileChange(WCHAR cDosName,LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr)
{
	return S_OK;
}

STDMETHODIMP CTestWindow::OnDiskSearch_Progress(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur)
{
	Post([=]
	{
		CString String; String.Format(_T("正在扫描:%f%%"), (float)dwCur/(float)dwTotalFile * 100);
		this->SetDlgItemText(IDC_STATIC_Result, String);
	});
	
	return S_OK;
}

STDMETHODIMP CTestWindow::OnDiskSearch_FileCountChange(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount)
{
	Post([=]
	{
		CString String; String.Format(_T("文件总数:%d"), dwFileCount + dwDirCount);
		this->SetDlgItemText(IDC_STATIC_Result, String);
	});
	return S_OK;
}

STDMETHODIMP CTestWindow::OnDiskSearch_StateChangeNotify(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam)
{
	LPCTSTR lpszCurState = _T("");
	switch( nMsg )
	{
	case DiskState_UnKnown: lpszCurState = _T("未知"); break;
	case DiskState_Ready:lpszCurState = _T("未知"); break;
	case DiskState_Scaning:lpszCurState = _T("正在扫描"); break;
	case DiskState_Finish:lpszCurState = _T("完成"); break;
	case DiskState_Failed:lpszCurState = _T("失败"); break;
		break;
	}

	Post([=]
	{
		this->SetDlgItemText(IDC_STATIC_State, lpszCurState);
	});
	return S_OK;
}

STDMETHODIMP CTestWindow::OnDiskSearch_Result(DWORD dwCount, DWORD dwTickCount)
{

	Post([=]
	{
		ListView_SetItemCount(m_dlTaskMgrListControl.m_hWnd, dwCount);
		ListView_RedrawItems(m_dlTaskMgrListControl.m_hWnd,0, 20);
	});

	return S_OK;
}

LRESULT CTestWindow::OnBnClickedButtonLoad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	if ( m_pDiskSearch )
	{
		MessageBox(_T("已加载"));
		return 0;
	}

	m_pRot->CreateInstance(CLSID_RpcDiskSearchCli, NULL, re_uuidof(IDiskSearchCli), (void**)&m_pDiskSearch);
	if ( !m_pDiskSearch )
	{
		MessageBox(_T("创建对象失败"));
		return 0;
	}

	m_pDiskSearch->Init( this);
	//m_pDiskSearch->LoadDisk();
	return 0;
}


LRESULT CTestWindow::OnBnClickedButtonUnload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	
	if ( m_pDiskSearch )
	{
		m_pDiskSearch->UnInit();
		m_pDiskSearch = INULL;
	}

	return 0;
}

LRESULT CTestWindow::OnEditNameChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	Query();

	return 0;
}

LRESULT CTestWindow::OnEditPathChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Query();
	return 0;
}



LRESULT CTestWindow::OnBnClickedCheckFullword(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	Query();
	return 0;
}


LRESULT CTestWindow::OnBnClickedCheckCase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	Query();
	return 0;
}


LRESULT CTestWindow::OnBnClickedCheckSubpath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	Query();
	return 0;
}
