// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

#include "mscom/mscominc.h"
using namespace msdk;

#include <mslog/stdlog_dll.h>
#define GroupName _T("fSearch")

#define MODULE_NAME GroupName
#include <mslog/msdkoutput.h>
#include <IfSearch.h>
#include <atlstr.h>
