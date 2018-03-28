// dllmain.cpp : DllMain 的实现。

#include "stdafx.h"
#include "resource.h"
#include "JCDeskBand_i.h"
#include "dllmain.h"
#include "..\code\DuiCore\DuiSkinFile.h"
#include "DuiDefine.h"
#include "DuiImpl\DuiObjectFactoryRegistHelp.h"

//CUIObjectFactoryRegistHelp


CJCDeskBandModule _AtlModule;
HINSTANCE g_instance = NULL;
//CPaintManagerUI m_pm;
// DLL 入口点
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_instance = hInstance;
	BOOL bRet = _AtlModule.DllMain(dwReason, lpReserved); 
	if ( dwReason == DLL_PROCESS_ATTACH)
	{
	}

	if ( dwReason == DLL_PROCESS_DETACH)
	{
	}


	return bRet;
}
