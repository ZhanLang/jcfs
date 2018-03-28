// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once


#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES


#include <atlbase.h>	// 基本的ATL类
#include <atlstr.h>
#include <atltypes.h>
#include <atlapp.h>		// 基本的WTL类


#include <atlwin.h>		// ATL窗口类
#include <atlcrack.h>	// WTL增强的消息宏
//#include <atlsplit.h>
#include <atlframe.h>	// WTL主框架窗口类
//#include <atlgdi.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlmisc.h>	// WTL实用工具类

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>
#include <com-cfg.h>




#include "resource.h"
#include "res\resource.h"
using namespace SOUI;

#include <mscom/mscominc.h>
using namespace mscom;

#define GroupName _T("JCfsUI")
#define MODULE_NAME GroupName
#include <mslog/stdlog_dll.h>
#include <mslog/msdkoutput.h>
#include <shlobj.h>
