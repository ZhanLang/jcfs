#include "stdafx.h"

namespace DuiLib{;

CTemplateUI::CTemplateUI(void)
{
	m_bVisible = false;
}


CTemplateUI::~CTemplateUI(void)
{
}

LPCTSTR CTemplateUI::GetClass() const
{
	return _T("Template");
}

LPVOID CTemplateUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("Templage")) == 0 ) return static_cast<CTemplateUI*>(this);
	return CControlUI::GetInterface(pstrName);
}

void CTemplateUI::SetTemplateXml(LPCTSTR sz)
{
	m_strTmpl = sz;
}

CDuiString CTemplateUI::GetTemplateXml()
{
	return m_strTmpl;
}

};
