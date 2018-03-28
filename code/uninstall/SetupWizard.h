#pragma once

#include "mscom/srvbase.h"
#include "luahost/ILuaHost.h"
#include "luahost/LuaHostPluginImpl.h"
#include "thread/SubThreadImp.h"
#include "ISetupWizard.h"
#include "arith/7Zip/7Zip.h"
class CSetupWizard:
	public IMsPlugin,
	public IMsPluginRun,
	public ISetupWizard,
	public CLuaPluginImpl,
	public CSubThread,
	public CMsComBase<CSetupWizard>
{
public:
	UNKNOWN_IMP4_(IMsPlugin, IMsPluginRun, ILuaPlugin,ISetupWizard);
	CSetupWizard(void);
	~CSetupWizard(void);


	STDMETHOD(Init)(void*);
	STDMETHOD(Uninit)();
	STDMETHOD(Start)();
	STDMETHOD(Stop)();

protected:
	HRESULT Run();

protected:
	STDMETHOD(SetWizardNotify)(ISetupWizardNotifty* pNotify);
	STDMETHOD_(BOOL, GetSilent)();
	STDMETHOD_(DWORD, GetOperID)();
	STDMETHOD_(INT, DoSetup)();
	STDMETHOD_(LPCTSTR, GetProgramPosition)();

	STDMETHOD_(LPCTSTR, GetProductName)();
	STDMETHOD_(LPCTSTR, GetFriendProdName)();
	STDMETHOD_(LPCTSTR, GetProuctId)();
	STDMETHOD_(LPCTSTR, GetVersion)();
	STDMETHOD_(LPCTSTR, GetMutex)();
	STDMETHOD_(BOOL, IsDebug)();

public:
	BEGIN_DEFLUAFUNCTION_MAP("Setup",CLuaPluginImpl)
		DEFINE_LUAFUNCTION_JSON("SetProgress", LuaSetProgress)
		DEFINE_LUAFUNCTION_JSON("GetControlCheckState", LuaGetControlCheckState)
		DEFINE_LUAFUNCTION_JSON_NO_IN_PARAM("GetProgramPosition", LuaGetProgramPosition)
		DEFINE_LUAFUNCTION_JSON_NO_IN_PARAM("GetSilent", LuaGetSilent)
	END_DEFLUAFUNCTION_MAP

private:
	virtual ILuaVM* GetLuaVM()
	{
		return m_pLuaVM;
	}

	//lua extern function
	BOOL LuaSetProgress(Json::Value inParam, Json::Value& outParam);
	BOOL LuaSetSilent(Json::Value inParam, Json::Value& outParam);
	BOOL LuaGetControlCheckState(Json::Value inParam, Json::Value& outParam);
	BOOL LuaSetProgramPosition(Json::Value inParam, Json::Value& outParam);
	BOOL LuaGetProgramPosition(Json::Value& outParam);
	BOOL LuaGetSilent(Json::Value& outParam);

	BOOL CheckOutFile();
	BOOL ExtractConfigFile();
	BOOL LoadConfigFile();
	BOOL UnCompress();
	BOOL LoadRot();
	BOOL DoSetupLua();

private:
	UTIL::com_ptr<ILuaVM> m_pLuaVM;
	UTIL::com_ptr<ISetupWizardNotifty> m_pNotify;
	UTIL::sentry<BYTE*, UTIL::default_array_sentry> m_pMemPacket;
	CMSComLoader m_mscomLoader;

	UTIL::com_ptr<IMsEnvMgr> m_pEnv;
	BOOL m_bSilent;
	DWORD m_dwOperID;
	CString	m_strProgramPosition;	
	CString m_strProductName;
	CString m_strFriendProdName;
	CString m_strProuctId;
	CString m_strVersion;
	CString m_strMutex;
	BOOL	m_bDebug;

	LZMA_HANDLE m_h7z; 
};

