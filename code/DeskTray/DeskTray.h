#pragma once
#include <IDeskTray.h>
#include <mscom/SrvBaseImp.h>
#include <tray/traysrvplugin.h>
#include <tray/traywndplugin.h>
#include <mscomhelper/UseConnectionPoint.h>

#include <JCDeskMsc.h>

class CDeskTray : 
	public CSrvPluginImp<CDeskTray>,
	public ITrayMenuConnectPoint,
	public ITrayMsgConnectPoint,
	public IExit
{
public:
	UNKNOWN_IMP5_( IMsPlugin , IMsPluginRun, IExit,ITrayMenuConnectPoint,ITrayMsgConnectPoint);
	CDeskTray(void);
	~CDeskTray(void);
public:

	

	//CSrvPluginImp
	virtual HRESULT OnAfterInit(void*);
	virtual HRESULT OnBeforeUnint();
	virtual HRESULT OnAfterStart();
	virtual HRESULT OnBeforeStop();
	virtual VOID WINAPI FireConnectBroken();
protected:
	//ITrayMenuConnectPoint
	STDMETHOD_(LRESULT, OnShowLeftButtonMenu)(INT x, INT y, BOOL& bHandle) ;		//左键菜单
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
	STDMETHOD(NotifyExit)(bool* bExit = NULL);

private:
	VOID DoReportActive();
private:
	UTIL::com_ptr<ITraySrv>					m_pTraySrv	; //托盘
	UseConnectPoint<ITrayMenuConnectPoint>  m_TrayMenuConnectPoint;
	UseConnectPoint<ITrayMsgConnectPoint>	m_TrayMsgConnectPoint;


private:

};

