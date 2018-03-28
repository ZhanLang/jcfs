// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

#include <mscom/mscominc.h>
using namespace mscom;

#include <mslog/stdlog_dll.h>
#define GroupName _T("VCDRomUI")

#include <UIlib.h>
using namespace DuiLib;

#include "resource.h"

#include <atlstr.h>
// TODO: 在此处引用程序需要的其他头文件

inline DuiLib::CControlUI* CreateControlElement(LPCTSTR lpXmlText, DuiLib::IDialogBuilderCallback* pCallback, DuiLib::CPaintManagerUI* pManager)
{
	CDuiString strHeader, strEnd;
	strHeader.Format(_T("%s"), _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<Window>\r\n"));
	strEnd.Format(_T("%s"), _T("</Window>"));
	DuiLib::CDialogBuilder builder;

	CDuiString strTemp = lpXmlText;
	strTemp = strHeader+strTemp+strEnd;
	lpXmlText = (LPCTSTR)strTemp;
	return builder.Create(lpXmlText, (UINT)0,  pCallback, pManager);
}


// TODO: 在此处引用程序需要的其他头文件
