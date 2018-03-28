// SSoft.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SSoft.h"

#include <shlobj.h>

#include <msapi/EnumFile.h>
#include "MyChiToLetter.h"


LPCTSTR szExcept[] = {
	_T("卸载"),
	_T("向导"),
	_T("设置"),
	_T("帮助"),
	_T("配置"),
	_T("UNINSTALL"),
	_T("HELP"),
};

LPCTSTR szRm[] = {
	_T("启动"),
};


CSSoft::CSSoft()
{
	CSubThread::StartThread();
}

CSSoft::~CSSoft()
{
	CSubThread::StopThread(TRUE);
}

DWORD CSSoft::GetMatch(LPCWSTR strMatch, DWORD dwMatch /*= SOFT_MATCH_AUTO*/)
{
	AUTOLOCK_CS( m_soft );

	for ( CSoft::iterator it = m_soft.begin() ; it != m_soft.end() ; it++ )
	{
		//首先匹配文件名称

	}

	return 0;
}

LPCWSTR CSSoft::GetLinkPath(DWORD dwIndex)
{
	return NULL;
}

LPCWSTR CSSoft::GetName(DWORD dwIndex)
{
	return NULL;
}

HRESULT CSSoft::Run()
{
	Build();

	while(TRUE)
	{
		DWORD dwWait = WaitForSingleObject(m_hExit, 5 * 1000);
		if ( dwWait == WAIT_OBJECT_0)
			break;
		
		if ( WAIT_TIMEOUT == dwWait )
			Build();
	}
	
	return 0;
}


BOOL FileEnumFunction(LPCTSTR lpszPath, LPVOID lpParam)
{

	TCHAR szLnkFile[ MAX_PATH ] = { 0 };
	CSSoft* pThis = (CSSoft*)lpParam;
	LPCTSTR lpszExt = PathFindExtension( lpszPath );

	//只找lnk程序
	if ( lpszExt && _tcsicmp( lpszExt, _T(".lnk")) != 0)
		return TRUE;

	CString szFileName = lpszPath;
	szFileName = szFileName.Left(szFileName.Find('.')).Mid(szFileName.ReverseFind('\\') + 1);

	CString strTemp = szFileName;
	strTemp.MakeUpper();

	for ( int nLoop = 0 ; nLoop < _countof(szExcept) ; nLoop++  )
	{
		if ( strTemp.Find(szExcept[nLoop]) != -1 )
			return TRUE;
	}

	//从名称中剔除一些干扰信息
	for ( int nLoop = 0 ; nLoop < _countof( szRm ) ; nLoop++)
	{
		szFileName.Replace(szRm[nLoop], _T(""));
	}

	if ( !szFileName.GetLength() )
		return TRUE;


	//删除前置空格
	while(TRUE)
	{
		int nPos = szFileName.Find(' ');
		if ( nPos != 0)
			break;

		szFileName.Delete(0);
	}


	
	//添加拼音
	CChiToLetter chi; chi.m_LetterEnd = TRUE; chi.m_blnFirstBig = TRUE; chi.m_blnAllBiG = FALSE;
	USES_CONVERSION;
	CStringA sNameA = T2A(szFileName);
	ST_SOFT pinyin = { lpszPath };
	pinyin.strPinyin =  chi.GetLetter( sNameA );
	chi.m_blnSimaple = TRUE;
	pinyin.strFrist = chi.GetLetter(sNameA);
		
	pinyin.strName = szFileName;
	pinyin.strLinkPath = lpszPath;
	
	pThis->Insert(pinyin);
	return TRUE;
}

BOOL CSSoft::Build()
{

	CoInitialize( NULL );

	int shGuid[] = { 
		CSIDL_COMMON_DESKTOPDIRECTORY, 
		CSIDL_DESKTOP,

		CSIDL_COMMON_STARTMENU,
		CSIDL_STARTMENU,
	};





	{
		AUTOLOCK_CS(m_soft);
		m_soft.clear();
	}
	
	
	for ( int nLoop = 0 ; nLoop < _countof(shGuid) ; nLoop++)
	{
		TCHAR szDeskTop[ MAX_PATH ] = { 0 };
		if ( SHGetSpecialFolderPath(NULL, szDeskTop, shGuid[nLoop], FALSE) )
			msapi::CEnumFile(FileEnumFunction, this ,TRUE).EnumFile(szDeskTop, _T("*.*"));
	}


	CoUninitialize();

	return TRUE;
}

BOOL CSSoft::Insert(ST_SOFT& soft)
{
	AUTOLOCK_CS( m_soft );

	for ( CSoft::iterator it = m_soft.begin() ; it != m_soft.end() ; it++ )
	{
		if ( soft.strName.CompareNoCase(it->strName) == 0)
			return TRUE;
	}


	m_soft.push_back(soft);
	return TRUE;
}
