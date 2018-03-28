#ifndef __UIANIMATROTATE_H__
#define __UIANIMATROTATE_H__

#pragma once

class CAnimatRotate : public CLabelUI
{
public:
	CAnimatRotate();
	~CAnimatRotate();

	void PaintBkImage(HDC hDC);

	void DoInit();
	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
protected:
	int m_nStep;
	Gdiplus::Bitmap *m_bitmap;
};

#endif // __UIANIMATROTATE_H__
