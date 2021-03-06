// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include <msapi/mssrv.h>
#include <msapi/msapp.h>
#include <msapi/mswinapi.h>
#include <msapi/mstime.h>
#include <thread/threadex.h>
#include <network/HttpImplement.h>
#include <userguid/GetNetAdapterInfo.h>
#include "MainDlg.h"
#include "SRotateWindow.h"
#include "JCfsUI.h"
#include "FsMcAdapterFix.h"
#include "WTLVirtualList.h"
#include "..\controls.extend\gif\SGifPlayer.h"

#include "SDlgSrvError.h"
#include "SFreeImageWnd.h"
#include "SShowImageWnd.h"
#include "SShowGifWnd.h"

//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE 1
//#define RES_TYPE 0   //从文件中加载资源
// #define RES_TYPE 1  //从PE资源中加载UI资源

CJCfsUI::CJCfsUI() : m_pComMgr( NULL ), m_pTheApp( NULL )
{

}

CJCfsUI::~CJCfsUI()
{

}

HRESULT CJCfsUI::OnAfterInit(void*)
{
	FreeImage_Initialise();
	//HRESULT hRes = OleInitialize(NULL);
	m_pComMgr = new SComMgr;

	BOOL bLoaded=FALSE;
	CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
	CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
	bLoaded = m_pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
	SASSERT_FMT(bLoaded,_T("load interface [render] failed!"));
	bLoaded=m_pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
	SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));

	pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
	m_pTheApp = new SApplication(pRenderFactory, GetModuleInstance());
	{
		CAutoRefPtr<IResProvider> sysResProvider;
		CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
		sysResProvider->Init((WPARAM)GetModuleInstance(), 0);
		m_pTheApp->LoadSystemNamedResource(sysResProvider);
	}

	m_pTheApp->RegisterWindowClass<SRotateWindow>();
	m_pTheApp->RegisterWindowClass<CWTLVirtualList>();
	m_pTheApp->RegisterWindowClass<SFreeImageWnd>();
	m_pTheApp->RegisterWindowClass<SShowImageWnd>();
	m_pTheApp->RegisterWindowClass<SShowGifWnd>();
	m_pTheApp->RegisterWindowClass<SGifPlayer>();//theApp中增加方法：RegisterWindowClass,替换RegisterWndFactory(TplSWindowFactory<SGifPlayer>())
	m_pTheApp->RegisterSkinFactory(TplSkinFactory<SSkinGif>());//注册SkinGif

	CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
	if (!pResProvider->Init((LPARAM)_T("uires"), 0))
	{
		SASSERT(0);
		return E_FAIL;
	}
#else 
	CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
	pResProvider->Init((WPARAM)GetModuleInstance(), 0);
#endif

	m_pTheApp->AddResProvider(pResProvider);
	
	m_pDlgMain = new CMainDlg( m_pRot );
	m_pDlgMain->Create(NULL, (DWORD)WS_POPUP, (DWORD)00);
	BOOL bRet = m_pDlgMain->SendMessage(WM_INITDIALOG);
	m_pDlgMain->CenterWindow(m_pDlgMain->m_hWnd);
	m_pDlgMain->ShowWindow(SW_SHOWNORMAL);

	
	if ( !( bRet && msapi::IsSrvExisted(_T("JCSrv")) && msapi::IsSrvRunning(_T("JCSrv"))) )
	{
		SDlgSrvError srvErrDlg;
		int nRet = srvErrDlg.DoModal(m_pDlgMain->m_hWnd);
		return E_FAIL;
	}
	else//只有服务启动的情况下才启动托盘
	{
		CThreadEx().Do([this] {//启动托盘
			if ( msapi::GetProcessIdAsName(_T("JCTray.exe")) == -1)
			{
				CString strJCTray;
				msapi::GetCurrentPath(strJCTray.GetBufferSetLength(MAX_PATH), MAX_PATH);
				strJCTray.ReleaseBuffer();
				strJCTray.Append(_T("\\JCTray.exe"));

				SHELLEXECUTEINFO shellInfo = { 0 };
				shellInfo.cbSize = sizeof( shellInfo );
				shellInfo.lpVerb = _T("open");
				shellInfo.hwnd = NULL;
				shellInfo.lpFile =(LPCWSTR)strJCTray;
				shellInfo.nShow = SW_SHOWNORMAL;
				shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
				ShellExecuteEx( &shellInfo );
			}
		});
	}


	//初始化
	if ( m_pDlgMain->IsWindow() )
	{
		
	}

	return S_OK;
}

HRESULT CJCfsUI::OnBeforeUnint()
{
	CFuncTime funcTime(_T("反初始化："), _T("OnBeforeUnint"));
	if ( m_pDlgMain )
	{
		delete m_pDlgMain;
		m_pDlgMain = NULL;
	}
	if ( m_pTheApp )
	{
		delete m_pTheApp;
		m_pTheApp = NULL;
	}

	if ( m_pComMgr )
	{
		delete m_pComMgr;
		m_pComMgr = NULL;
	}
	FreeImage_DeInitialise();
	return S_OK;
}

HRESULT CJCfsUI::OnAfterStart()
{
	CThreadEx().Do([this]{DoReportActive();});
	m_pTheApp->Run(m_pDlgMain->m_hWnd);
	return S_OK;
}

HRESULT CJCfsUI::OnBeforeStop()
{
	return S_OK;
}

VOID WINAPI CJCfsUI::FireConnectBroken()
{
	TerminateProcess( GetCurrentProcess(),0);
}

HRESULT CJCfsUI::OnMscDeskTrayExit(RSMSGID dwMessageID,IMsBuffer* lpXMessage)
{
	if (m_pDlgMain && m_pDlgMain->IsWindow() )
	{
		m_pDlgMain->Post([=]{m_pDlgMain->OnClose();});
	}
	
	return S_OK;
}

VOID CJCfsUI::DoReportActive()
{
	msdk::network::CUrlParamValueMap ParamValueMap;
	msapi::CApp m_App(CSrvPluginImp::GetEnvParamString("productname"));
	ParamValueMap["apiver"]		=		1;					//当前API版本
	ParamValueMap["hdd"]			=		m_App.GetDiskSerial();
	ParamValueMap["mac"]			=		CGetNetAdapterInfo::GetMac();
	ParamValueMap["osver"]		=		msapi::GetMicroVersion();
	ParamValueMap["wow64"]	=		msapi::IsWindowsX64();
	ParamValueMap["prod"]		=		m_App.GetSetting(_T("prod"),0);
	ParamValueMap["packet"]		=		m_App.GetSetting(_T("packet"),0);
	ParamValueMap["subprod"] =		CSrvPluginImp::GetEnvParamString("subprod");	//当前模块的子产品
	ParamValueMap["idate"]		=		m_App.GetSetting(_T("idate"), msapi::GetCurrentDateStr(FALSE));
	ParamValueMap["itime"]		=		m_App.GetSetting(_T("itime"), msapi::GetCurrentTimeStr(FALSE));
	ParamValueMap["prodver"]	=		m_App.GetVersion();

	msdk::network::CHttpImplement HttpImpl;
	HttpImpl.GetRequest("update.kyan.com.cn", 80, "active" , ParamValueMap);
}

