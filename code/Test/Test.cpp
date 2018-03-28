// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..\JCfsClient\JCfsClient_i.h"
#include "..\JCSrv\JCSrv_i.h"
#include "mscom\msbase.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize( NULL );

// 	CComQIPtr<ICJfsSrv> m_CJfsSrv;
// 	RFAILED(CoCreateInstance(CLSID_CJfsSrv, NULL, CLSCTX_LOCAL_SERVER, IID_ICJfsSrv, (void**)&m_CJfsSrv));
// 	RASSERT( m_CJfsSrv, E_FAIL);
// 	RFAILED(m_CJfsSrv->SetCallBack(CComPtr<IUnknown>(m_CJfsSrv)));

	USES_GUIDCONVERSION;
	LPCTSTR lposz = GUID2S(CLSID_CJCfsClient);

	

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0 , 0))
	{
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}

	return 0;
}

