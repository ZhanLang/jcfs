// dllmain.h : 模块类的声明。

class CJCfsClientModule : public ATL::CAtlDllModuleT< CJCfsClientModule >
{
public :
	DECLARE_LIBID(LIBID_JCfsClientLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_JCFSCLIENT, "{1CC0C8C3-EE8E-4795-9AEC-4CFEA3E2A9F5}")
};

extern class CJCfsClientModule _AtlModule;
