// SouiInit.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SouiInit.h"
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif


CSouiInit::CSouiInit():
m_pComMgr(NULL),
	m_theApp(NULL)
{

}


CSouiInit::~CSouiInit()
{

}

SComMgr* CSouiInit::GetSouiComMgr()
{
	return m_pComMgr;
}

SApplication* CSouiInit::GetSouiApplication()
{
	return m_theApp;
}

STDMETHODIMP CSouiInit::Init(void*)
{
	m_pComMgr = new SComMgr;
	BOOL bLoaded=FALSE;
	CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
	CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
	bLoaded = m_pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
	SASSERT_FMT(bLoaded,_T("load interface [render] failed!"));
	bLoaded=m_pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
	SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));


	pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

	m_theApp = new SApplication(pRenderFactory, g_hinstance);

	//从DLL加载系统资源
	HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
	if (hModSysResource)
	{
		CAutoRefPtr<IResProvider> sysResProvider;
		CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
		sysResProvider->Init((WPARAM)hModSysResource, 0);
		m_theApp->LoadSystemNamedResource(sysResProvider);
		FreeLibrary(hModSysResource);
	}else
	{
		SASSERT(0);
	}

	CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
	if (!pResProvider->Init((LPARAM)_T("skin\\deskband_skin"), 0))
	{
		SASSERT(0);
		return 1;
	}
#else 
	CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
	pResProvider->Init((WPARAM)hInstance, 0);
#endif

	//m_theApp->InitXmlNamedID(namedXmlID,ARRAYSIZE(namedXmlID),TRUE);
	m_theApp->AddResProvider(pResProvider);

	//加载多语言翻译模块。
	CAutoRefPtr<ITranslatorMgr> trans;
	bLoaded=m_pComMgr->CreateTranslator((IObjRef**)&trans);
	SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("translator"));
	if(trans)
	{//加载语言翻译包
		m_theApp->SetTranslator(trans);
		pugi::xml_document xmlLang;
		if(m_theApp->LoadXmlDocment(xmlLang,_T("lang_cn"),_T("translator")))
		{
			CAutoRefPtr<ITranslator> langCN;
			trans->CreateTranslator(&langCN);
			langCN->Load(&xmlLang.child(L"language"),1);//1=LD_XML
			trans->InstallTranslator(langCN);
		}
	}
	return S_OK;
}

STDMETHODIMP CSouiInit::Uninit()
{
	return S_OK;
}
