// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#define  _WTL_NO_CSTRING	
#include "resource.h"
#include <atlbase.h>
#include <atlapp.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlmisc.h>

#include <WindowsX.h>

#define MSUI_STATIC
#include <DuiApi.h>
using namespace DuiKit;
extern HINSTANCE g_instance;

//extern CPaintManagerUI m_pm;