#include "StdAfx.h"
#include "WebBaidu.h"

#include <network/HttpImplement.h>
#include <json/json.h>
#include <util/stdstringextend.h>
using namespace network;

CWebBaidu::CWebBaidu(void)
{
	m_hSemaphore = CreateSemaphore(NULL, 0, 1000, NULL);
	CSubThread::StartThread();
}


CWebBaidu::~CWebBaidu(void)
{
	CSubThread::StopThread();
	CSubThread::WaitForSubThreadExit();
}


STDMETHODIMP_(CLSID) CWebBaidu::GetClsid()
{
	return CLSID_WebBaidu;
}

STDMETHODIMP CWebBaidu::SetNotify(ISWebNotify* pNotify)
{
	RASSERT( pNotify , E_INVALIDARG);
	m_pSwebNotify = pNotify;
	return m_pSwebNotify ? S_OK : E_INVALIDARG;
}
STDMETHODIMP CWebBaidu::GetSuggest(LPCWSTR lpszSuggest)
{
	RASSERT( m_pSwebNotify, E_FAIL);
	RASSERT( lpszSuggest && lpszSuggest[0], E_INVALIDARG);
	m_strText = lpszSuggest;
	if ( !ReleaseSemaphore(m_hSemaphore, 1, NULL) )
		return E_FAIL;
	

	return S_OK;
}

HRESULT CWebBaidu::Run()
{
	HANDLE hHandles[] = { m_hExit, m_hSemaphore};

	for ( ; ; )
	{
		DWORD dwWait = WaitForMultipleObjects(_countof(hHandles), hHandles, FALSE, INFINITE);
		switch( dwWait )
		{
		case WAIT_OBJECT_0://m_hExit
			{
				GrpMsg(GroupName, MsgLevel_Msg, _T("CWebBaidu Run Exit"));
				return S_OK;
			}
		case WAIT_OBJECT_0 + 1://hExp
			{
				if ( m_pSwebNotify )
					m_pSwebNotify->OnSWebNotify_Msg( SWEB_MSG_BEGIN, NULL);

				CUrlParamValueMap valueMap;
				valueMap["wd"] = m_strText.GetBuffer();

				CHttpImplement httpImpl;
				DWORD dwRet = httpImpl.GetRequest("suggestion.baidu.com", 80, "su",valueMap);

				if ( dwRet )
				{
					if ( m_pSwebNotify )
						m_pSwebNotify->OnSWebNotify_Msg( SWEB_MSG_BEGIN, NULL);

					break;
				}


				
				std::string sValue = (char*)httpImpl.GetRequestData();
				sValue = sValue.substr(sValue.find('[') + 1);
				sValue = sValue.erase(sValue.find(']'));

				sValue = strext::replace<char>(sValue.c_str(), "\"", "").c_str();
				std::vector<std::string> sVect = strext::split<char>(sValue.c_str(), ",");

				UTIL::com_ptr<IProperty2> pProp;
				m_pRot->CreateInstance(CLSID_CProperty2, NULL, __uuidof(IProperty2), (void**)&pProp);

				if ( !pProp )
				{
					if ( m_pSwebNotify )
						m_pSwebNotify->OnSWebNotify_Msg( SWEB_MSG_BEGIN, NULL);

					break;
				}

				
				CPropSet propSet(pProp);
				for ( int nLoop = 0 ; nLoop < sVect.size() ; nLoop++ )
					propSet[nLoop] = sVect[nLoop].c_str();


				if ( m_pSwebNotify )
				{
					m_pSwebNotify->OnSWebNotify_Suggest(this, sVect.size(), pProp);
					m_pSwebNotify->OnSWebNotify_Msg( SWEB_MSG_DONE, NULL);
				}
				
				break;
			}
		}
	}
}
