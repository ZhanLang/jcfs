#pragma once


#include "mscom/srvbase.h"
#include "ISetupWizard.h"
#include "thread/SubThreadImp.h"
class CDuiWindow :
	public IMsPlugin,
	public IMsPluginRun,
	public ISetupWizardNotifty,
	public WindowImplBase,
	public CMsComBase<CDuiWindow>,
	public CSubThread
{
public:
	UNKNOWN_IMP3_(IMsPlugin, IMsPluginRun, ISetupWizardNotifty);

	CDuiWindow(void);
	~CDuiWindow(void);

	STDMETHOD(Init)(void*);
	STDMETHOD(Uninit)();
	STDMETHOD(Start)();
	STDMETHOD(Stop)();

	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const ;
	CDuiString GetZIPFileName() const;
	virtual LPCTSTR GetResourceID() const;

protected:
	virtual HRESULT	Run();
protected:
	//DUI_DECLARE_MESSAGE_MAP()
	virtual void OnInitialization(TNotifyUI& msg);
	virtual void OnClick(TNotifyUI& msg);
protected:
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void InitWindow();

protected:
	STDMETHOD_(VOID, OnSetProgress)(INT nProgress);
	STDMETHOD_(VOID, OnError )(ErrorType errorType); //安装失败
	STDMETHOD_(VOID, OnFinesh)();	//安装成功
	STDMETHOD_(VOID, OnExtractPercent)(INT nProgress);
	STDMETHOD_(VOID, OnStep)(StepType step);


	STDMETHOD_(BOOL, GetControlCheckState)(LPCTSTR lpszControlName) ;
	STDMETHOD_(CString, GetProgramPosition)();
private:
	UTIL::com_ptr<ISetupWizard> m_pSetupWizard;
};


// {937E6A54-D431-41F2-AE07-FBCAE057E03F}
MS_DEFINE_GUID(CLSID_SetupWindows, 
	0x937e6a54, 0xd431, 0x41f2, 0xae, 0x7, 0xfb, 0xca, 0xe0, 0x57, 0xe0, 0x3f);
