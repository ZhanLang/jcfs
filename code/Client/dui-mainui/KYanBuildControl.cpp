#include "StdAfx.h"
#include "KYanBuildControl.h"
#include "WTLVirtualList.h"

#include "GifControlUI.h"
#include "KYImageControl.h"



CKYanBuildControl::CKYanBuildControl(void)
{
}


CKYanBuildControl::~CKYanBuildControl(void)
{
}


CControlUI* CKYanBuildControl::CreateControl(LPCTSTR pstrClass)
{
	if(_tcscmp(pstrClass, _T("WTLVirtualList")) == 0)
	{
			return new CWTLVirtualList;
	}
	if (_tcscmp(pstrClass , _T("Gif")) == 0)
	{
		return new CGifControlUI;
	}
	if (_tcscmp(pstrClass, _T("KYanImageControl")) == 0)
	{
		return new CKYImageControl;
	}
	return NULL;
}


