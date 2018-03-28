#pragma once
#include "UIlib.h"
using namespace DuiLib;

class CKYImageControl : public DuiLib::CControlUI
{
public:
	CKYImageControl(void);
	~CKYImageControl(void);

	void	SetPathFile(LPCTSTR lpszFile);
};

