// dllmain.h : 模块类的声明。

class CJCDeskBandModule : public ATL::CAtlDllModuleT< CJCDeskBandModule >
{
public :
	DECLARE_LIBID(LIBID_JCDeskBandLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_JCDESKBAND, "{A0734B89-26C9-418E-AAB6-A8973458B11C}")
};

extern class CJCDeskBandModule _AtlModule;
