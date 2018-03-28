#pragma once

#include <ISWeb.h>
#include <thread/SubThreadImp.h>
class CWebHaosou:
	public ISWeb, 
	public CMsComBase<CWebHaosou>,
	public CSubThread
{
public:
	UNKNOWN_IMP1( ISWeb);
	CWebHaosou(void);
	~CWebHaosou(void);


protected://ISWeb
	STDMETHOD_(CLSID, GetClsid)();
	STDMETHOD(SetNotify)( ISWebNotify* pNotify);
	STDMETHOD(GetSuggest)( LPCWSTR lpszSuggest);

	virtual HRESULT	Run();

private:
	UTIL::com_ptr<ISWebNotify> m_pSwebNotify;
	HANDLE m_hSemaphore;

	CString m_strText;
};


