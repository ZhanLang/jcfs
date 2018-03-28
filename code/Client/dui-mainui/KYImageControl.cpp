#include "StdAfx.h"
#include "KYImageControl.h"

#include "ImageEx.h"
CKYImageControl::CKYImageControl(void)
{
}


CKYImageControl::~CKYImageControl(void)
{
}

void CKYImageControl::SetPathFile(LPCTSTR lpszFile)
{
	CImageEx image;
	RECT rect = {0};
	if (image.LoadFromFile(lpszFile))
	{
		int dwParentHeight = this->GetParent()->GetHeight();
		int dwParentWidth = this->GetParent()->GetWidth();

		if (dwParentHeight - image.GetHeight() > 0)
		{
			this->SetMaxHeight(image.GetHeight());
			rect.top = (dwParentHeight - this->GetMaxHeight()) / 2;
		}
		else
		{
			this->SetMaxHeight(dwParentHeight);
		}
		if (dwParentWidth - image.GetHeight() > 0)
		{
			this->SetMaxWidth(image.GetWidth());
			rect.left = (dwParentWidth - this->GetMaxWidth()) /2;
		}
		else
		{
			this->SetMaxWidth(dwParentWidth);
		}
	}

	this->SetPadding(rect);

	SetBkImage(lpszFile);
	Invalidate();
}
