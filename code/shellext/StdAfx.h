// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__5DBE34B3_1481_41EF_992E_B17B731DFD8D__INCLUDED_)
#define AFX_STDAFX_H__5DBE34B3_1481_41EF_992E_B17B731DFD8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED

#define _ATL_STATIC_REGISTRY

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#include <winternl.h>
#include "atlstr.h"
#include "mscom/mscominc.h"
#include "mslog/logtool/logtool.h"
using namespace mscom;


#define GroupName _T("ShellExt")



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5DBE34B3_1481_41EF_992E_B17B731DFD8D__INCLUDED)
