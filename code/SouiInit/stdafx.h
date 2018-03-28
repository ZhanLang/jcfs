// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>
#include <com-cfg.h>
#define R_IN_CPP	//定义这个开关来
using namespace SOUI;

// TODO: 在此处引用程序需要的其他头文件

#include <mscom/mscominc.h>
using namespace mscom;

#define GroupName _T("DeskTray")
#include <mslog/stdlog_dll.h>

#include <desktray/IDeskTray.h>