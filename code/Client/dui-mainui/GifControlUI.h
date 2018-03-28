#pragma once
#include "UIlib.h"
using namespace DuiLib;
#include "GifImage.h"
class CGifControlUI:
	public DuiLib::CControlUI
{
public:
	CGifControlUI(void);
	~CGifControlUI(void);

	virtual LPCTSTR GetClass() const;
	virtual LPVOID GetInterface(LPCTSTR pstrName);
	virtual UINT GetControlFlags() const;
	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoInit();

	void	SetPathFile(LPCTSTR lpszFile);
	virtual void PaintBkImage(HDC hDC);
	virtual void Event(TEventUI& event);
	virtual void SetVisible(bool bVisible = true);
// 	virtual int GetWidth() const;
// 	virtual int GetHeight() const;
private:
	CDuiString m_strGif;
	CGifImage  m_gifImage;
	int m_nFramePos;
};

