
#pragma once



enum ErrorType
{
	ErrorType_CheckOut = 2,	  //检出文件失败
	ErrorType_UnCompress, //解压缩失败
	ErrorType_LoadRot,	  //加载安装程序失败
	ErrorType_DoScript,	  //执行脚本失败	
};


enum StepType
{
	StepType_CheckOut,
	StepType_UnCompress,
	StepType_DoScript,
	StepType_Done,
};

struct ISetupWizardNotifty : public IMSBase
{
	STDMETHOD_(BOOL, GetControlCheckState)(LPCTSTR lpszControlName) = 0;
	STDMETHOD_(CString, GetProgramPosition)() = 0;
	
	STDMETHOD_(VOID, OnStep)(StepType step) = 0;

	STDMETHOD_(VOID, OnExtractPercent)(INT nProgress) = 0;

	STDMETHOD_(VOID, OnSetProgress)(INT nProgress) = 0;
	STDMETHOD_(VOID, OnError )(ErrorType errorType) = 0; //安装失败
	STDMETHOD_(VOID, OnFinesh)() = 0;	//安装成功
};

MS_DEFINE_IID(ISetupWizardNotifty, "{32220B59-ED77-4524-9394-761E9643DA7C}");


struct ISetupWizard : public IMSBase
{
	STDMETHOD(SetWizardNotify)(ISetupWizardNotifty* pNotify) = 0;
	STDMETHOD_(LPCTSTR, GetProgramPosition)() = 0;
	STDMETHOD_(LPCTSTR, GetProductName)() = 0;
	STDMETHOD_(LPCTSTR, GetFriendProdName)() = 0;
	STDMETHOD_(LPCTSTR, GetProuctId)() = 0;
	STDMETHOD_(LPCTSTR, GetVersion)() = 0;
	STDMETHOD_(LPCTSTR, GetMutex)() = 0;
	//是否静默状态
	STDMETHOD_(BOOL, GetSilent)() = 0;
	STDMETHOD_(DWORD, GetOperID)() = 0;
	STDMETHOD_(INT, DoSetup)() = 0;

	STDMETHOD_(BOOL, IsDebug)() = 0;
};

MS_DEFINE_IID(ISetupWizard, "{3D14D1B0-3358-4C99-A180-59472026A117}");


// {5F812410-F284-4EA0-B51A-9E0711614540}
MS_DEFINE_GUID(CLSID_SetupWizard, 
	0x5f812410, 0xf284, 0x4ea0, 0xb5, 0x1a, 0x9e, 0x7, 0x11, 0x61, 0x45, 0x40);
