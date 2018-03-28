#pragma once
#include <functional>
#include <thread/worker.h>
#include <download/IDownload.h>

#include <tray/traysrvplugin.h>
#include <tray/traywndplugin.h>
#include <mscomhelper/UseConnectionPoint.h>


class CUpdataUI : 
	public IMsPlugin,
	public IMsPluginRun,
	public IDownloadNotify,
	public ITrayMenuConnectPoint,
	public ITrayMsgConnectPoint,
	public worker::IWorkerDispatcher,
	public WindowImplBase,
	public CMsComBase<CUpdataUI>
	
{
public:
	CUpdataUI();
	UNKNOWN_IMP5_(IMsPlugin, IMsPluginRun,IDownloadNotify,ITrayMenuConnectPoint,ITrayMsgConnectPoint )
	STDMETHOD(Init)(void*);
	STDMETHOD(Uninit)();
	STDMETHOD(Start)();
	STDMETHOD(Stop)();

	//DuiLib
protected:

	virtual CDuiString GetZIPFileName() const;
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	
	virtual LPCTSTR GetResourceID() const;
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	CControlUI* CreateControl(LPCTSTR pstrClass);
protected:
	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT , WPARAM , LPARAM , BOOL& bHandled);
	virtual LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	//DuiLib
//	DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);
		virtual void OnWinInit(TNotifyUI& msg);
		virtual void OnSelChange(TNotifyUI& msg);
		virtual void OnTabSel(TNotifyUI& msg);
		//virtual void OnItemDBClick(TNotifyUI& msg);


protected:
	virtual void OnWorkStatus(LPARAM lParam);

	void Post(const std::function<void()> &);

protected:
	STDMETHOD(OnDownloadNotify)(DL_NotifyEvnet notifyType, DWORD dwTaskId);

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

private:
	VOID ShowPage(INT nPage);
	VOID SetDlProgress( INT prog);
	VOID DlFinish();
	worker::CWorkCenter m_workCenter;

	CString m_strUrl;
	CString m_strName;
	CString m_strSave;
	CString m_strVer;

	CString m_strSkin;
	UTIL::com_ptr<IDownloadMgr> m_pDlMgr;
	BOOL m_bDownLoad;
	INT  m_dlProg;
	INT  m_nTaryIcon;
	UTIL::com_ptr<ITraySrv>					m_pTraySrv	; //托盘
	UseConnectPoint<ITrayMenuConnectPoint>  m_TrayMenuConnectPoint;
	UseConnectPoint<ITrayMsgConnectPoint>	m_TrayMsgConnectPoint;

};

// {4B5C8284-0B63-460E-A469-111BEB3B475A}
MS_DEFINE_GUID(CLSID_UpdataUI, 
	0x4b5c8284, 0xb63, 0x460e, 0xa4, 0x69, 0x11, 0x1b, 0xeb, 0x3b, 0x47, 0x5a);
