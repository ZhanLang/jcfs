#pragma once


enum SWEB_MSG
{
	SWEB_MSG_BEGIN,
	SWEB_MSG_DONE,
	SWEB_MSG_ERROR,
};

struct ISWeb;
struct ISWebNotify : public IMSBase
{
	STDMETHOD(OnSWebNotify_Msg)(SWEB_MSG SWebMsg, LPARAM lParam) = 0;
	STDMETHOD(OnSWebNotify_Suggest)(ISWeb*pSweb, DWORD dwCount, IProperty2* pResult) = 0;
};
MS_DEFINE_IID(ISWebNotify, "{74605F56-3E95-4A1E-B822-708DCD9F3C54}");


struct ISWeb : public IMSBase
{
	STDMETHOD_(CLSID, GetClsid)() = 0;
	STDMETHOD(SetNotify)( ISWebNotify* pNotify) = 0;
	STDMETHOD(GetSuggest)( LPCWSTR lpszSuggest) = 0;	//获取推荐信息
};

MS_DEFINE_IID(ISWeb, "{C0779AF7-3B87-4A52-B18A-2251DD981532}");

// {CF4DF126-DD4A-4447-8170-CE377C30522B}
MS_DEFINE_GUID(CLSID_WebBaidu, 
	0xcf4df126, 0xdd4a, 0x4447, 0x81, 0x70, 0xce, 0x37, 0x7c, 0x30, 0x52, 0x2b);


// {E5A04954-1FA8-4791-8C7F-1B70A2753B78}
MS_DEFINE_GUID(CLSID_WebSogou, 
	0xe5a04954, 0x1fa8, 0x4791, 0x8c, 0x7f, 0x1b, 0x70, 0xa2, 0x75, 0x3b, 0x78);


// {7F443335-F9F2-4C33-95DC-8A60ACBF49AB}
MS_DEFINE_GUID(CLSID_WebHaosou, 
	0x7f443335, 0xf9f2, 0x4c33, 0x95, 0xdc, 0x8a, 0x60, 0xac, 0xbf, 0x49, 0xab);
