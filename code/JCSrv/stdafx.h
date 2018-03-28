// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_FREE_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlstr.h>
using namespace ATL;


#include <mscom/mscominc.h>
using namespace mscom;

#include <mslog/stdlog_dll.h>
#define GroupName L"JCSrv"

#define MODULE_NAME GroupName
#include <mslog/msdkoutput.h>
#include "SrvModule.h"
extern CSrvModule _AtlModule;