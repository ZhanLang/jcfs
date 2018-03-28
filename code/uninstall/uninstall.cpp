// uninstall.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "uninstall.h"
#include <shlwapi.h>
#include <crash/DumpFile.h>
#include "msapi/mswinapi.h"

#include "DuiWindow.h"
#include "SetupWizard.h"
#include <mslog/msdkoutput.h>
HINSTANCE  g_hinstance = NULL;
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	DeclareDumpFile();
	g_hinstance = hInstance;

	UTIL::com_ptr<ISetupWizard> pSetupWizard;
	DllQuickCreateInstance(CLSID_SetupWizard, __uuidof(ISetupWizard), pSetupWizard, NULL);
	RASSERT(pSetupWizard, -1);

	{
		CFuncTime func(_T("_tWinMain"), _T("ISetupWizard::Init"));
		UTIL::com_ptr<IMsPlugin> pPlugin = pSetupWizard;
		RFAILEDP(pPlugin->Init(NULL), -1);
	}


	BOOL bDebug = pSetupWizard->IsDebug();
	
	//静默安装模式
	if (pSetupWizard->GetSilent()){
		CFuncTime func(_T("_tWinMain"), _T("ISetupWizard::DoSetup"));
		return pSetupWizard->DoSetup();
	}
	else{
		UTIL::com_ptr<IMsPlugin> pSetupWindowPlugin;
		DllQuickCreateInstance(CLSID_SetupWindows, __uuidof(IMsPlugin), pSetupWindowPlugin.m_p, NULL);
		if( !pSetupWindowPlugin ) return -1;

		{
			CFuncTime func(_T("_tWinMain"), _T("CLSID_SetupWindows::Init"));
			RFAILEDP(pSetupWindowPlugin->Init(pSetupWizard.m_p), -1);
		}


		UTIL::com_ptr<IMsPluginRun> pSetupWindowPluginRun = pSetupWindowPlugin;
		RFAILEDP(pSetupWindowPluginRun->Start(), -1);

		{
			CFuncTime func(_T("_tWinMain"), _T("CLSID_SetupWindows::Uninit"));
			pSetupWindowPlugin->Uninit();
		}


	}

	{
		CFuncTime func(_T("_tWinMain"), _T("ISetupWizard::Uninit"));
		UTIL::com_ptr<IMsPlugin> pPlugin = pSetupWizard;
		pPlugin->Uninit();
	}



	pSetupWizard = INULL;
	if ( !bDebug )
	{
		CFuncTime func(_T("_tWinMain"), _T("DeletePathFile"));
		//mspath::CPath::DeleteFileEx(strTempPath);
		//msapi::DeletePathFile(strTempPath);
	}

	return 0;
}

BEGIN_CLIDMAP
	CLIDMAPENTRY_BEGIN
	CLIDMAPENTRY_NOROT(CLSID_SetupWindows,CDuiWindow)
	CLIDMAPENTRY_NOROT(CLSID_SetupWizard, CSetupWizard)
	CLIDMAPENTRY_END
	END_CLIDMAP_AND_EXPORTFUN;