#pragma once
#include <mscom/srvbase.h>
#include <ISouiInit.h>
class CSouiInit : public ISouiInit , public IMsPlugin , public CMsComBase<CSouiInit>
{
public:
	UNKNOWN_IMP2_( ISouiInit, IMsPlugin);

	STDMETHOD(Init)(void*);
	STDMETHOD(Uninit)();

	CSouiInit();
	~CSouiInit();

	virtual SComMgr* GetSouiComMgr();
	virtual SApplication* GetSouiApplication();

private:
	SComMgr * m_pComMgr;
	SApplication *m_theApp;
};