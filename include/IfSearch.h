#pragma once

//前置声明
namespace msdk{
	namespace mscom{
		struct IMsDwordVector;
	};};

/*打开操作掩码*/
enum{
	OpenDiskMask_UseCache			= ( 1 << 1 ),			//使用Cache缓存,lpszNameSpace为空则默不使用Cache
	OpenDiskMask_DeleteCacheFile  = (OpenDiskMask_UseCache << 1), //删除以前CACHE的文件
};


/*文件操作掩码*/
enum{//OnFileChange
	FileAction_New,		//创建文件
	FileAction_Del,		//删除文件
	FileAction_Rm,		//重命名
	FileAction_OldRm,	//重命名前的名称
};


/*磁盘当前的状态*/
enum //OnDiskSearch_StateChangeNotify
{
	DiskState_UnKnown,
	DiskState_Ready,	//准备就绪
	DiskState_Scaning,	//正在扫描
	DiskState_Finish,	//完成
	DiskState_Failed,	//失败

	//一下是服务端使用的
	DiskState_Mount,	//挂载
	DiskState_uMount,	//卸载
};

/*文件级别*/
enum
{
	FileLeave_0,	
	FileLeave_1,
	FileLeave_2,
	FileLeave_3,
	FileLeave_4,
	FileLeave_5,
	FileLeave_6,
	FileLeave_7,
};


//查询条件掩码
enum
{
	QueryConditionMask_File		=	  (1)	<< 1,	//查找文件
	QueryConditionMask_Dir		=	  (1)	<< 2,	//查找目录
	QueryConditionMask_Case		=	  (1)	<< 4,	//大小写敏感
	QueryConditionMask_Full		=	  (1)	<< 5,	//全字匹配
	QueryConditionMask_Sub		=	  (1)	<< 6,	//目录过滤的时候是否包含子目录，如果不设置，则不包含。
	
	QueryConditionMask_Exclude_System =		(1)	<< 7,	//排除系统文件
	QueryConditionMask_Exclude_Hidden =		(1)	<< 8,	//排除隐藏的文件
	QueryConditionMask_FullExt		  =		(1)	<< 9,	//完整匹配后缀
};

struct QueryCondition
{
	QueryCondition()
	{
		dwSize = 0;
		dwConditionMask = /*QueryConditionMask_File | QueryConditionMask_Dir*/0;
		lpstrIncludeDir = _T("");
		lpstrExtension  = _T("");
		lpstrFileName   = _T("");
	}

	DWORD	dwSize;				//sizeof(QueryCondition)
	DWORD	dwConditionMask;	//查询条件掩码 QueryConditionMask_All

	//需要搜索的文件夹:如果为空，则默认搜索当前搜索引擎所在的根目录下
	LPCWSTR lpstrIncludeDir;		//包含的路径,以(|)分割;其中每个路径均以'\'结尾
	LPCWSTR lpstrExtension;		//过滤扩展名,以(|)分割

	LPCWSTR	lpstrFileName;		//按文件名称查询，支持模糊匹配
};


//查询结果索引
struct QUERY_RESULT_INDEX
{
	QUERY_RESULT_INDEX(CHAR d, BOOL f, CHAR l, DWORD i):
		Volume(d),IsFile(f),Leave(l),Index(i){}

	QUERY_RESULT_INDEX(DWORD r):Result(r){}

	union {
		DWORD Result;
		struct {
			DWORD Volume 	: 5;	//驱动器
			DWORD IsFile   	: 1; //是否为文件
			DWORD Leave    	: 3; //优先级
			DWORD Index	   	: 23;//索引
		};
	};
};



enum 
{
	GetValueMask_IcoIndex		= (1)	<<	1,	//获取图标索引
	GetValueMask_FileSize		= (1)	<<	2,	//获取文件大小
	GetValueMask_MondifyTime	= (1)	<<	3,	//获取修改时间
	GetValueMask_CreateTime		= (1)	<<	4,	//获取创建时间
	GetValueMask_AccessTime		= (1)	<<	5,	//获取访问时间
	GetValueMask_Attributes		= (1)	<<	6,	//获取属性
};

struct SearchResult
{
	WCHAR		szName[MAX_PATH];			//名称
	WCHAR		szPath[MAX_PATH];			//路径
	WCHAR		szExtension[MAX_PATH];		//扩展名
	
	ULONGLONG	ullFileSize;				//文件大小

	ULONGLONG	ullMondifyTime;				//修改时间
	ULONGLONG	ullCreateTime;				//创建时间
	ULONGLONG	ullAccessTime;				//访问时间

	DWORD		dwFileAttr;					//文件属性

	DWORD		dwLeave;
	DWORD		dwIconIndex;				//图标索引
};

struct IDiskSearch;
struct IDiskSearchNotify : public IMSBase
{
	//文件发生变动
	STDMETHOD(OnDiskSearch_FileChange)(WCHAR cDosName, LPCWSTR lpFile, DWORD dwAction, DWORD dwAttr) = 0;

	//进度报告
	STDMETHOD(OnDiskSearch_Progress)(WCHAR cDosName, DWORD dwTotalFile, DWORD dwCur) = 0;

	//文件总数发生变化
	STDMETHOD(OnDiskSearch_FileCountChange)(WCHAR cDosName, DWORD dwFileCount, DWORD dwDirCount) = 0;

	//状态发生变化
	STDMETHOD(OnDiskSearch_StateChangeNotify)(WCHAR cDosName, INT nMsg, WPARAM wParam, LPARAM lParam) = 0;

	STDMETHOD(OnDiskSearch_Result)( DWORD dwCount, DWORD dwTickCount) = 0;
};

MS_DEFINE_IID(IDiskSearchNotify, "{7E40E819-CD28-4816-88B7-493EDBCBC35D}");



struct IDiskSearch : public IMSBase
{
	STDMETHOD(OpenDisk)(WCHAR cDosName, IDiskSearchNotify* pNotify, DWORD OpenDiskMask = OpenDiskMask_UseCache) = 0;
	STDMETHOD(CloseDisk)() = 0;

	STDMETHOD(LoadDisk)() = 0;
	STDMETHOD(UnLoadDisk)() = 0;

	STDMETHOD_(WCHAR, GetVolumeName)() = 0;
	STDMETHOD_(DWORD, GetCurrentState)() = 0;

	STDMETHOD(LockDisk)() = 0;
	STDMETHOD(UnLockDisk)() = 0;
	STDMETHOD_(BOOL, IsLockDisk)() = 0;

	STDMETHOD(Query)(const QueryCondition& Condition, IMsDwordVector* pDwordVector) = 0;
	STDMETHOD(GetResultAt)( DWORD dwIndex, DWORD GetValueMask,SearchResult& info) = 0;

	STDMETHOD_(DWORDLONG, GetCount)(DWORD* dwFileCount, DWORD* dwDirCount) = 0;


	STDMETHOD(SetFileMark)(LPCWSTR lpszFile, DWORD dwLeave) = 0;
};

MS_DEFINE_IID(IDiskSearch, "{96A167C1-2CAB-47D6-A4BF-BAE83547B045}");

// {6AD7BD55-3181-48B5-9528-11F3C932B8A7}
MS_DEFINE_GUID(CLSID_Ntfs, 
	0x6ad7bd55, 0x3181, 0x48b5, 0x95, 0x28, 0x11, 0xf3, 0xc9, 0x32, 0xb8, 0xa7);

// {EADA0E2D-E66A-4E83-BBB6-4EDE5920892C}
MS_DEFINE_GUID(CLSID_Fat, 
	0xeada0e2d, 0xe66a, 0x4e83, 0xbb, 0xb6, 0x4e, 0xde, 0x59, 0x20, 0x89, 0x2c);




//////////////////////////////////////////////////////////////////////////
//文件搜索服务组件
struct IDiskSearchSrv : public IMSBase
{
	STDMETHOD_(DWORD, GetSearchDisk)(IMSBase /*IProperty2*/** pSerchDisk) = 0;
	STDMETHOD(GetSearchDisk)(WCHAR cDosName, IMSBase/*IDiskSearch*/** pDisk) = 0;
	STDMETHOD(Lock)() = 0;
	STDMETHOD(UnLock)() = 0;
	STDMETHOD(SetFileMark)(LPCWSTR lpszFile, DWORD dwLeave) = 0;

	STDMETHOD_(DWORD,GetState)() = 0;
	STDMETHOD(ReBuildIndex)() = 0;
};

MS_DEFINE_IID(IDiskSearchSrv,"{9D361F24-1B63-4E5D-98A6-373A992A4F4A}");


// {B4EFD843-3001-4602-9B3E-1BA2245DB6F7}
MS_DEFINE_GUID(CLSID_DiskSearchSrv, 
	0xb4efd843, 0x3001, 0x4602, 0x9b, 0x3e, 0x1b, 0xa2, 0x24, 0x5d, 0xb6, 0xf7);

//文件搜索客户端组件
struct IDiskSearchCli : public IMSBase
{
	STDMETHOD(Init)(IDiskSearchNotify* pClientNotify) = 0;
	STDMETHOD(UnInit)() = 0;

	STDMETHOD(Query)(QueryCondition& queryCondition) = 0;
	STDMETHOD(GetResultAt)(DWORD dwIndex, DWORD GetValueMask, SearchResult& ResultInfo)  = 0;

	STDMETHOD_(DWORD, GetState)() = 0;

	STDMETHOD(SetFileMark)(LPCWSTR lpszFile, DWORD dwLeave) = 0;
};

MS_DEFINE_IID(IDiskSearchCli, "{AE68E005-4E7A-4495-BF58-8131744C7379}");

// {11EBD2AD-DC63-496B-8359-561072B9C009}
MS_DEFINE_GUID(CLSID_DiskSearchCli, 
	0x11ebd2ad, 0xdc63, 0x496b, 0x83, 0x59, 0x56, 0x10, 0x72, 0xb9, 0xc0, 0x9);


//RPC 过程调用接口
// {505E90B5-222A-4C5D-94B9-0FE39621503D}
MS_DEFINE_GUID(CLSID_RpcDiskSearchCli,
	0x505E90B5,0x222A,0x4C5D,0x94,0xB9,0x0F,0xE3,0x96,0x21,0x50,0x3D);