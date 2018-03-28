#pragma once
#include <ISWeb.h>
#include <thread/SubThreadImp.h>

class CWebSogou:
	public ISWeb, 
	public CMsComBase<CWebSogou>,
	public CSubThread
{
public:
	UNKNOWN_IMP1( ISWeb);
	CWebSogou(void);
	~CWebSogou(void);


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

