#pragma once

#include <tray/traysrvplugin.h>
#include <mscom/SrvBaseImp.h>
#include <mscomhelper/UseConnectionPoint.h>
#include <signalapp/ISignalApp.h>

#include <commx/comx3helper.h>


#include <UIlib.h>
#include "WTLVirtualList.h"
#include <CommonControls.h>

#include "SyncObject/criticalsection.h"
#include "shellcontextmenu/ShellContextMenu.h"

#include <set>

using namespace DuiLib;




struct menu_command
{
	CString strShow;
	CString strValue;
};
typedef std::vector<menu_command>	CPathMenuCommand;


#define NPC_FILTER_PATH_MESSAGE (WM_USER + 888)



class CDuiWindow:
	public ITrayMenuConnectPoint,
	public ITrayMsgConnectPoint,
	public IExit,
	public ISingleAppMsgConnectPoint,
	public CSrvPluginImp<CDuiWindow>,
	public CWindowWnd, 
	public INotifyUI,
	public CComx3Helper<CDuiWindow>
{
public:
	UNKNOWN_IMP7_( IMsPlugin , IMsPluginRun, IExit,ISingleAppMsgConnectPoint,ITrayMenuConnectPoint,ITrayMsgConnectPoint,IMessageCallBack);
// 	BEGIN_COMX3_MSG_MAP(CDuiWindow)
// 		COMX3_DEFID_MADKID_BEGIN(COMX3_SHELL_CONTEXTMENU_DEFID, COMX3_SHELL_CONTEXTMENU_MASKID)
// 			DEFINE_COMX3_MSG(COMX3_SHELL_CONTEXTMENU_SEARCH_MSG, OnShellContextMenuComX3Message)
// 		COMX3_DEFID_MADKID_END
// 	END_COMX3_MSG_MAP
public:
	CDuiWindow(void);
	~CDuiWindow(void);

protected:
	HRESULT OnAfterInit(void*);
	HRESULT OnBeforeUnint();
	HRESULT OnAfterStart();
	HRESULT OnBeforeStop();

protected:
	//ITrayMenuConnectPoint
	STDMETHOD_(LRESULT, OnShowLeftButtonMenu)(INT x, INT y, BOOL& bHandle) ;			//左键菜单
	STDMETHOD_(LRESULT, OnShowRightButtonMenu)(INT x, INT y, BOOL& bHandle);		//右键菜单
	STDMETHOD_(LRESULT, OnMenuCommand)(WORD nMenuId, BOOL &bHandle);
	STDMETHOD_(LRESULT, OnMenuCommand)(UINT nMenuIndex, HMENU hMenu, BOOL &bHandle);
	STDMETHOD_(LRESULT, OnDefaultMenu)(BOOL &bHandle);	//左键双击响应默认菜单

protected:
	//ITrayMsgConnectPoint
	STDMETHOD_(LRESULT, OnTrayIconMsg)(LPARAM lParam, INT x, INT y, BOOL& bHandle);	//托盘图标消息
	STDMETHOD_(LRESULT, OnBalloonClicked)(UINT nActionID, BOOL& bHandle);				//气球被点击
	STDMETHOD_(LRESULT, OnOtherMsg)(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandle); //其他消息
protected:
	STDMETHOD(OnSingleAppMsg)(LPCWSTR lpArgXml , BOOL& bHandle);



protected:
	STDMETHOD(NotifyExit)(bool* bExit = NULL);
protected:

	virtual HRESULT OnShellContextMenuComX3Message(const CLIENTINFO& clientInfo, MESSAGEINFO* pMessageInfo, IMessage*pMessage);
protected:
	
	void OnHeaderDbClick(TNotifyUI& msg);
	void OnAddBtnClick(TNotifyUI& msg);
	void OnCloseTable(TNotifyUI& msg);
	void OnTableSelectChange(TNotifyUI& msg);


	void OnMinBtnClick(TNotifyUI& msg);
	void OnMaxBtnClick(TNotifyUI& msg);
	void OnRestoreBtnClick(TNotifyUI& msg);
	void OnCloseBtnClick(TNotifyUI& msg);
	void OnAd_LayoutClick(TNotifyUI& msg);

	void OnEditTextChange(TNotifyUI& msg);

protected:
	LPCTSTR GetWindowClassName() const ;
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND /*hWnd*/);
	void Notify(TNotifyUI& msg);
	BOOL ShowActiveWindow();
	void OnClick(TNotifyUI& msg);

protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT nID, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled);
	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelectPathBtnClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNpcFilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	LRESULT OnMenuVisitWebsite(WORD id, WORD code);
	LRESULT OnMenuRestoreWindow(WORD id, WORD code);
	LRESULT OnMenuExit(WORD id, WORD code);
	LRESULT OnMenuReBuildIndex(WORD id, WORD code);
	LRESULT OnTrayMenuSearhFolder(WORD id, WORD code);
	LRESULT OnTrayMenuMonitorFileChange(WORD id, WORD code);
private:
	
	HRESULT InitMenu();
	HRESULT InitMenu(CSkinMenu& skinMenu);
	
	BOOL CreateTableItem(LPCTSTR lpszPath);

private:

private:
	private:
	UTIL::com_ptr<ITraySrv>					m_pTraySrv	; //托盘
	UTIL::com_ptr<IMSBase>					m_pSignalApp;
	UseConnectPoint<ITrayMenuConnectPoint>  m_TrayMenuConnectPoint;
	UseConnectPoint<ITrayMsgConnectPoint>	m_TrayMsgConnectPoint;
	UseConnectPoint<ISingleAppMsgConnectPoint>	m_SingleAppConnectPoint;

	

	UTIL::com_ptr<IStringW> m_pComX3NameSpace;
	
	CPathMenuCommand					m_pathMenuCmd;
private:
public:
	CPaintManagerUI m_pm;

private:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CButtonUI* m_pSearch;
private:

	LPBYTE m_lpResourceZIPBuffer;
	CString						m_strFilterPathFormNpc;
};

// {376E1DA4-7DFC-4a3e-8134-2D47666DEB45}
MS_DEFINE_GUID(CLSID_SearchUI, 
			   0x376e1da4, 0x7dfc, 0x4a3e, 0x81, 0x34, 0x2d, 0x47, 0x66, 0x6d, 0xeb, 0x45);