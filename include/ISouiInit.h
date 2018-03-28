#pragma once

struct ISouiInit : public IMSBase
{
	virtual SComMgr* GetSouiComMgr() = 0;
	virtual SApplication* GetSouiApplication() = 0;
};

MS_DEFINE_IID(ISouiInit, "{05C1EF71-F7A3-4AE8-8B92-F3EB174219B2}");

// {018C3F45-9B8F-402F-9C62-B6B36479E857}
MS_DEFINE_GUID(CLSID_SouiInit, 
	0x18c3f45, 0x9b8f, 0x402f, 0x9c, 0x62, 0xb6, 0xb3, 0x64, 0x79, 0xe8, 0x57);
