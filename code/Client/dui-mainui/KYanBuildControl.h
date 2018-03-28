#pragma once

#include "UIlib.h"
using namespace DuiLib;
class CKYanBuildControl: public IDialogBuilderCallback
{
public:
	CKYanBuildControl(void);
	~CKYanBuildControl(void);
	
public:
	CControlUI* CreateControl(LPCTSTR pstrClass);
};

