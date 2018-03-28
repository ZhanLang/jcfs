#pragma once

#include <mscom/SrvBaseImp.h>
#include "MainDlg.h"
#include <IfSearch.h>
#include <combase/IMsVector.h>
#include <JCDeskMsc.h>
class CJCfsUI : public CSrvPluginImp<CJCfsUI>
{
public:
	CJCfsUI();
	~CJCfsUI();

	UNKNOWN_IMP2_( IMsPlugin , IMsPluginRun);

	//MSCÏûÏ¢¶©ÔÄ
	MSC_BEGIN_DEFINE_MSG(CDeskTray)
		MSC_DEFINE_MSG(MSCMSG_JCDESK_TRAY_EXIT)
	MSC_END_DEFINE_MSG


	MSC_BEGIN_MSG_MAP(CDeskTray)
		MSC_MSG_MAP(MSCMSG_JCDESK_TRAY_EXIT, OnMscDeskTrayExit)
	MSC_END_MSG_MAP;

	virtual HRESULT OnAfterInit(void*);
	virtual HRESULT OnBeforeUnint();
	virtual HRESULT OnAfterStart();
	virtual HRESULT OnBeforeStop();
	virtual VOID WINAPI FireConnectBroken();

protected:
	 HRESULT OnMscDeskTrayExit(RSMSGID dwMessageID,IMsBuffer* lpXMessage);

private:
	BOOL InitFS();

	VOID DoReportActive();

private:
	SComMgr *m_pComMgr;
	SApplication *m_pTheApp;

	CMainDlg* m_pDlgMain;

	

};


// {419B2D05-37E6-4852-BCA3-CD40AD325032}
MS_DEFINE_GUID(CLSID_JCfsUI, 
	0x419b2d05, 0x37e6, 0x4852, 0xbc, 0xa3, 0xcd, 0x40, 0xad, 0x32, 0x50, 0x32);
