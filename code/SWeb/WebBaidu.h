#pragma once
#include <ISWeb.h>

#include <thread/SubThreadImp.h>
class CWebBaidu : 
	public ISWeb, 
	public CMsComBase<CWebBaidu>,
	public CSubThread
{
public:
	UNKNOWN_IMP1( ISWeb);
	CWebBaidu(void);
	~CWebBaidu(void);


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

