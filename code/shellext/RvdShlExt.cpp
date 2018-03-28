// JCfsShlExt.cpp : Implementation of CRvdShlExt
#include "stdafx.h"
#include "RvdExt.h"
#include "RVDShlExt.h"


#include <mslog/logtool/logtool.h>
#include <process/ParseCommand.h>
/////////////////////////////////////////////////////////////////////////////
// CRvdShlExt

#include <atlcom.h>
#include <atlconv.h>

#define  RVD_BLANK 1
#define  RVD_DRIVE 2

/////////////////////////////////////////////////////////////////////////////
// CRvdShlExt

INT CRvdShlExt::GetRvdPath(LPTSTR lpBuff, INT nLen)
{
#ifdef DEBUG
	lstrcpy(lpBuff, _T("D:\\work\\appproduct2011\\app\\rvd\\src\\rvdext\\"));
#else
    return 0;
#endif
	/*if(!lpBuff || !nLen) return 0;
	ZeroMemory(lpBuff, nLen);

	BOOL bFind = FALSE;
	#ifdef WIN32
	// 确定 配置文件位置
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
	_T("Software\\Rising\\Rvd"),
	0,KEY_READ,&hKey);
	if(ERROR_SUCCESS == lRet)
	{
	DWORD dwType = REG_SZ;
	DWORD dwSize = nLen * sizeof(TCHAR);
	lRet = RegQueryValueEx( hKey, _T("installpath"), NULL, &dwType,  (LPBYTE)lpBuff, &dwSize);
	if (lRet == ERROR_SUCCESS)
	{
	if(lpBuff[0] && lpBuff[_tcslen(lpBuff) - 1] != _T('\\'))
	_tcscat(lpBuff, _T("\\"));
	}
	else
	lpBuff[0] = 0;
	RegCloseKey(hKey);

	// 		if( strcmp(lpBuff,"") )
	// 		{
	// 			char rsMain[MAX_PATH]={0};
	// 			sprintf(rsMain,"%s\\RsMain.exe",lpBuff);
	// 			if( GetFileAttributes(rsMain) != INVALID_FILE_ATTRIBUTES)
	// 			{
	// 				bFind = TRUE;
	// 			}
	// 		}
	}

	#endif

	#ifdef WIN64
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
	_T("Software\\Rising\\Rvd"),
	0,KEY_READ|KEY_WOW32_64KEY,&hKey);
	if(ERROR_SUCCESS == lRet)
	{
	DWORD dwType = REG_SZ;
	DWORD dwSize = nLen * sizeof(TCHAR);
	lRet = RegQueryValueEx( hKey, _T("installpath"), NULL, &dwType,  (LPBYTE)lpBuff, &dwSize);
	if (lRet == ERROR_SUCCESS)
	{
	if(lpBuff[0] && lpBuff[_tcslen(lpBuff) - 1] != _T('\\'))
	_tcscat(lpBuff, _T("\\"));
	}
	else
	lpBuff[0] = 0;

	RegCloseKey(hKey);
	// 
	// 		if( strcmp(lpBuff,"") )
	// 		{
	// 			char rsMain[MAX_PATH]={0};
	// 			sprintf(rsMain,"%s\\RsMain.exe",lpBuff);
	// 			if( GetFileAttributes(rsMain) != INVALID_FILE_ATTRIBUTES)
	// 			{
	// 				bFind = TRUE;
	// 			}
	// 		}

	}
	// 
	// 	if(! bFind)
	// 	{
	// 		ZeroMemory(lpBuff, nLen * sizeof(TCHAR));
	// 		hKey = NULL;
	// 		lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
	// 			_T("Software\\Rising\\Rav"),
	// 			0,KEY_READ|KEY_WOW32_64KEY,&hKey);
	// 		if(ERROR_SUCCESS == lRet)
	// 		{
	// 			DWORD dwType = REG_SZ;
	// 			DWORD dwSize = nLen * sizeof(TCHAR);
	// 			lRet = RegQueryValueEx( hKey, _T("installpath"), NULL, &dwType,  (LPBYTE)lpBuff, &dwSize);
	// 			if (lRet == ERROR_SUCCESS)
	// 			{
	// 				if(lpBuff[0] && lpBuff[_tcslen(lpBuff) - 1] != _T('\\'))
	// 					_tcscat(lpBuff, _T("\\"));
	// 			}
	// 			else
	// 				lpBuff[0] = 0;
	// 
	// 			RegCloseKey(hKey);
	// 		}
	// 	}
	// 
	// #endif

	return _tcslen(lpBuff);*/
	return 1;
}
CRvdShlExt::CRvdShlExt()
{

	OutputDebugString("CRvdShlExt::CRvdShlExt()");
	

	TCHAR szPath[MAX_PATH]={0}; 
	ZeroMemory(m_szRvdBmpFile,MAX_PATH);
	GetRvdPath(m_szRvdBmpFile, sizeof(m_szRvdBmpFile));
  
	lstrcpy(szPath,m_szRvdBmpFile);
	_tcscat(m_szRvdBmpFile, _T("rvd.bmp"));

	m_hBmp= (HBITMAP)LoadImage(NULL, 
		m_szRvdBmpFile, 
		IMAGE_BITMAP , 0 , 0 , 
		LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	if ( !m_hBmp )
		m_hBmp = (HBITMAP)LoadImage(_Module.GetModuleInstance(), 
		MAKEINTRESOURCE(IDB_MENU), 
		IMAGE_BITMAP , 0 , 0 , 
		LR_DEFAULTCOLOR | LR_DEFAULTSIZE);

	//////////////////////////////////////////////////////////////////////////
	lstrcpy(szPath,m_szRvdBmpFile);
	_tcscat(m_szRvdBmpFile, _T("create.bmp"));

	m_hBmpCreate= (HBITMAP)LoadImage(NULL, 
		m_szRvdBmpFile, 
		IMAGE_BITMAP , 0 , 0 , 
		LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	if ( !m_hBmpCreate )
		m_hBmpCreate = (HBITMAP)LoadImage(_Module.GetModuleInstance(), 
		MAKEINTRESOURCE(IDB_BITMAP_SHELLMENU_KYAN), 
		IMAGE_BITMAP , 0 , 0 , 
		LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
}

CRvdShlExt::~CRvdShlExt()
{
	if(m_hBmp)
	{
		DeleteObject(m_hBmp);
		m_hBmp = NULL;
	}

	if(m_hBmpCreate)
	{
		DeleteObject(m_hBmpCreate);
		m_hBmpCreate = NULL;
	}
}

HRESULT CRvdShlExt::Initialize ( LPCITEMIDLIST pidlFolder,
								   LPDATAOBJECT pDataObj,
								   HKEY hProgID )
{

	//创建NPC对象
	OutputDebugString(_T("HRESULT CRvdShlExt::Initialize"));
	TCHAR buf[MAX_PATH ];
	if (SHGetPathFromIDList ( pidlFolder, buf ) )//在空白处点击右键
	{
		m_flag=RVD_BLANK;
		m_strPath=buf;
		if (m_strPath.GetLength()!=3)  //是在非根目录上右击
			m_strPath+=_T("\\");

		return S_OK;
	}

	//以下情况说明不是在空白处右击，要判断在磁盘或文件上右键的有效性

	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };
	HDROP     hDrop;


	if ( FAILED( pDataObj->GetData ( &fmt, &stg )))
	{

		return E_INVALIDARG;
	}

	hDrop = (HDROP) GlobalLock ( stg.hGlobal );

	if ( NULL == hDrop )
	{
		return E_INVALIDARG;
	}

	UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );

	if ( 0 == uNumFiles )
	{
		GlobalUnlock ( stg.hGlobal );
		ReleaseStgMedium ( &stg );
		return E_INVALIDARG;
	}

	HRESULT beNeed = S_OK;//是否需要卸载或加载自己的虚拟磁盘
	if ( 0 == DragQueryFile ( hDrop, 0, m_szFile, MAX_PATH ))
	{
		beNeed = E_INVALIDARG;
	}

	m_strPath=m_szFile;            //是在磁盘上或文件上右击

	if (m_strPath.GetLength()==3)  //是在磁盘上右击
	{
		CString strPathDisk = m_strPath.Left(2);
		TCHAR szName[MAX_PATH];
		if (QueryDosDevice(strPathDisk, szName, MAX_PATH))
		{
			strPathDisk=szName;
			if(strPathDisk.Find(_T("\\RsDisk")) >= 0)//在自己的磁盘上右击
				m_flag=RVD_DRIVE;
			else
				beNeed=E_INVALIDARG;                 //在其他磁盘上右击
				//bNeedCreate = TRUE;
		}
	}
	//else                  //是在文件上右击
	//{
	//if (m_strPath.Find(".rvd")>=0)//在自己的磁盘文件上右击
	// 	{
	// 		m_flag=RVD_FILE;
	// 	}
	//   else
		//hr=E_INVALIDARG;//在其他的磁盘文件上右击
	//}
	GlobalUnlock ( stg.hGlobal );
	ReleaseStgMedium ( &stg );

	//if(!bNeedCreate)
		//hr = E_INVALIDARG;

	return beNeed;
}

HRESULT CRvdShlExt::QueryContextMenu ( HMENU hmenu, UINT  uMenuIndex, 
										 UINT  uidFirstCmd, UINT  uidLastCmd,
										 UINT  uFlags )
{
	if ( uFlags & CMF_DEFAULTONLY )
	{
		return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );
	}

	DWORD dwFileAttr = GetFileAttributes(m_strPath);

	CString strKyanPath;
	msapi::GetModulePath(_Module.GetModuleInstance(), strKyanPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strKyanPath.ReleaseBuffer();
	strKyanPath.Append(_T("\\..\\JCfs.exe"));

	if( PathFileExists( strKyanPath ) )
	{
		m_szText.LoadString(IDS__FILE_SEARCH);
		InsertMenu ( hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, m_szText);
		SetMenuItemBitmaps(hmenu, uMenuIndex, MF_BITMAP|MF_BYPOSITION , m_hBmpCreate, m_hBmpCreate);
	}
// 	if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY || 
// 		dwFileAttr & FILE_ATTRIBUTE_DEVICE)
// 	{
		
	//}

// 	switch(m_flag)
// 	{
// 	case RVD_BLANK:
// 		m_szText.LoadString(IDS__FILE_CREATE);
// 		InsertMenu ( hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, m_szText);
// 		SetMenuItemBitmaps(hmenu, uMenuIndex, MF_BITMAP|MF_BYPOSITION , m_hBmpCreate, m_hBmpCreate);
// 		break;
// 	case RVD_DRIVE:
// 		m_szText.LoadString(IDS_FILE_UNMOUNT);
// 		InsertMenu ( hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, m_szText);
// 		SetMenuItemBitmaps(hmenu, uMenuIndex, MF_BITMAP|MF_BYPOSITION , m_hBmp, m_hBmp);
// 		break;
// // 	case RVD_FILE:
// // 		m_szText=_T("加载虚拟加密盘镜像文件");
// // 		InsertMenu ( hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, m_szText);
// // 		SetMenuItemBitmaps(hmenu, uMenuIndex, MF_BITMAP|MF_BYPOSITION , m_hBmp , m_hBmp);
// // 		break;
// 	}

	return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 1 );
}

HRESULT CRvdShlExt::GetCommandString ( UINT_PTR  idCmd,      UINT uFlags,
										 UINT* pwReserved, LPSTR pszName,
										 UINT  cchMax )
{
	USES_CONVERSION;
	
	if ( 0 != idCmd )
		return E_INVALIDARG;
	
	if ( uFlags & GCS_HELPTEXT )
	{

		if ( uFlags & GCS_UNICODE )
		{	
			lstrcpynW ( (LPWSTR) pszName, T2CW(m_szText), cchMax );
		}
		else
		{
			lstrcpynA ( pszName, T2CA(m_szText), cchMax );
		}

		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT CRvdShlExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo )
{

	if ( 0 != HIWORD( pCmdInfo->lpVerb ))
		return E_INVALIDARG;

	
	CString strKyanPath;
	msapi::GetModulePath(_Module.GetModuleInstance(), strKyanPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strKyanPath.ReleaseBuffer();
	strKyanPath.Append(_T("\\..\\JCfs.exe"));


	CParseCommand cmd;
	cmd.SetArg(_T("filter"), m_strPath);
	CString strCmd = cmd.GetCmdLine().c_str();
	
	SHELLEXECUTEINFO shExecInfo   = { 0 };
	shExecInfo.cbSize             = sizeof(shExecInfo);
	shExecInfo.lpFile             = strKyanPath;// _T("explorer.exe");
	shExecInfo.lpParameters       = strCmd.GetBuffer();//strCmd.GetBuffer();
	shExecInfo.nShow              = SW_SHOWNORMAL;
		//shExecInfo.lpVerb             = _T("open"); // Context menu item
	shExecInfo.fMask              = SEE_MASK_NOCLOSEPROCESS;
	BOOL bRet = ShellExecuteEx(&shExecInfo);

		
	switch(m_flag)
	{
	case RVD_BLANK:
       // m_diskOperate.CreateVdiskFile(m_strPath.GetBuffer());
		break;
	case RVD_DRIVE:
		//m_diskOperate.UMountVdisk(m_strPath.GetBuffer());
      	break;
 	//case  RVD_FILE:
 		//MessageBox(NULL,m_szText,m_szText,MB_OK);
		//break;
	default:
		return E_INVALIDARG;
		break;
	}
  return S_OK;
}
