#pragma once


#include "resource.h"
#include <atlctrls.h>
#include <atlframe.h>
#include "atlgdix.h"
#include "nfresize.h"
#define __ATLGDIX_H__
#include "atlgdi.h"

#include <IfSearch.h>
#include <functional>

#include <combase/IMsVector.h>


#define WM_EXECUTE_MSG (WM_USER + 55648)

class CTestWindow:
	public IExit,
	public IMsPlugin,
	public IMsPluginRun,
	public IDiskSearchNotify,
	public CAxDialogImpl<CTestWindow>,
	public CNoFlickerDialogResize<CTestWindow>,
	public CUpdateUI<CTestWindow>,
	public CMessageFilter, 
	public CIdleHandler,
	public CMsComBase<CTestWindow>
{
public:
	CTestWindow(void);
	~CTestWindow(void);
	enum{IDD = IDD_MAIN_FRAME};
	UNKNOWN_IMP4_(IMsPluginRun, IMsPlugin, IExit,IDiskSearchNotify);
	BEGIN_MSG_MAP(CTestWindow)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
		MESSAGE_HANDLER(WM_CLOSE, OnClose);
		MESSAGE_HANDLER(WM_SIZE, OnSize);
		//MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_EXECUTE_MSG, OnExecute)
		NOTIFY_HANDLER(IDC_DL_TASK_MGR_LIST, LVN_GETDISPINFO, OnResultListGetDispinfo)
			//MESSAGE_HANDLER(DOWNLOAD_TASK_MESSAGE,OnDownLoadMsg);
			//COMMAND_HANDLER(IDC_StartAll_Btn,BN_CLICKED,OnStartAllTask);
			//COMMAND_HANDLER(IDC_StopAll_Btn,BN_CLICKED,OnStopAllTask);
			//NOTIFY_HANDLER(IDC_DL_TASK_MGR_LIST, NM_RCLICK, OnNMRclickListLeak);
		COMMAND_HANDLER(IDC_BUTTON_LOAD, BN_CLICKED, OnBnClickedButtonLoad)
		CHAIN_MSG_MAP(CUpdateUI<CTestWindow>)
		CHAIN_MSG_MAP(CNoFlickerDialogResize<CTestWindow>)
			
		COMMAND_HANDLER(IDC_BUTTON_UNLOAD, BN_CLICKED, OnBnClickedButtonUnload)
		COMMAND_HANDLER(IDC_EDIT_NAME, EN_CHANGE, OnEditNameChange)
		COMMAND_HANDLER(IDC_EDIT_PATH, EN_CHANGE, OnEditPathChange)
		COMMAND_HANDLER(IDC_CHECK_FULLWORD, BN_CLICKED, OnBnClickedCheckFullword)
		COMMAND_HANDLER(IDC_CHECK_CASE, BN_CLICKED, OnBnClickedCheckCase)
		COMMAND_HANDLER(IDC_CHECK_SUBPATH, BN_CLICKED, OnBnClickedCheckSubpath)
	END_MSG_MAP();

	BEGIN_DLGRESIZE_MAP(CTestWindow)
		//BEGIN_DLGRESIZE_GROUP()
			//DLGRESIZE_CONTROL(IDC_DLTASKMGR_DLG_STATIC, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		//END_DLGRESIZE_GROUP()

 		DLGRESIZE_CONTROL(IDC_DL_TASK_MGR_LIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
// 		DLGRESIZE_CONTROL(IDC_GLOBAL_STATIC,DLSZ_MOVE_Y)
// 		DLGRESIZE_CONTROL(IDC_StartAll_Btn, DLSZ_MOVE_X|DLSZ_MOVE_Y)
// 		DLGRESIZE_CONTROL(IDC_StopAll_Btn,DLSZ_MOVE_X|DLSZ_MOVE_Y)
// 		DLGRESIZE_CONTROL(IDC_DelAll_Btn,DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_UPDATE_UI_MAP(CTestWindow)
	END_UPDATE_UI_MAP()


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCommand(UINT , WPARAM wParam, LPARAM , BOOL& bHandled);
	LRESULT OnTimer(UINT , WPARAM wParam, LPARAM , BOOL& bHandled);

	LRESULT OnExecute(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/);
	LRESULT OnResultListGetDispinfo(INT n, LPNMHDR lpNmhdr, BOOL& bHandle);
private:
	BOOL GetSearchExtension(CString& strText);
	BOOL GetSearchText(CString& strText);

public:
	void Post(const std::function<void()> &f);
	void Query();
protected:
	STDMETHOD(Init)(void*);
	STDMETHOD(Uninit)();
	STDMETHOD(Start)();
	STDMETHOD(Stop)();
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

protected:
	STDMETHOD(OnDiskSearch_FileChange)(WCHAR cDosName,LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr);
	STDMETHOD(OnDiskSearch_Progress)(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur);;
	STDMETHOD(OnDiskSearch_FileCountChange)(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount);
	STDMETHOD(OnDiskSearch_StateChangeNotify)(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(OnDiskSearch_Result)( DWORD dwCount, DWORD dwTickCount);
protected:
	STDMETHOD(NotifyExit)(bool* bExit = NULL);

	CListViewCtrl m_dlTaskMgrListControl;
	UTIL::com_ptr<IDiskSearchCli> m_pDiskSearch;
	UTIL::com_ptr<IMsDwordVector> m_pResult;
public:
	LRESULT OnBnClickedButtonLoad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonUnload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditNameChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditPathChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckFullword(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckCase(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckSubpath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


private:
	BOOL m_bSubPath;
	BOOL m_bFullWord;
	BOOL m_bFullCase;
	DWORD m_dwIndex;
	SearchResult m_Result;

CString m_strextension ;
};


// {CC9FB5DA-CC83-4874-A396-F6A1AB11B955}
MS_DEFINE_GUID(CLSID_TestWindow, 
	0xcc9fb5da, 0xcc83, 0x4874, 0xa3, 0x96, 0xf6, 0xa1, 0xab, 0x11, 0xb9, 0x55);
