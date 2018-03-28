#pragma once
//字符串处理类
const int ALPHABETA_SIZE = 0x80; /**/
extern BYTE g_NoCaseTable[ALPHABETA_SIZE]; /*加快字符串大小写转换*/

#pragma pack(push)
#pragma pack(1)

struct JC_FILE_BASIC_INFO{
	JC_FILE_BASIC_INFO( DWORD FileReferenceNumber)
	{
		Index = FileReferenceNumber;
	}

	JC_FILE_BASIC_INFO()
	{
	}

	union{
		struct{
			DWORD NoNascii			: 1;	//是否含有中文
			DWORD TwoByteNameLen	: 1;	//0表示文件名长占1字节 1表是文件名长占2字节
			DWORD ExtensionLen		: 2;	//表示扩展名字节数0 1 4
			DWORD Leave				: 3;	//文件级别
			DWORD IsSystem			: 1;	//系统文件
			DWORD IsHide			: 1;	//隐藏文件
			DWORD Index				: 23;	//文件索引
		};
		DWORD BasicInfo;
	};
};
/*
struct JC_DIRECTORY_BASIC_INFO{
	union {
		struct {
			DWORD IsSystem			: 1;	//系统文件
			DWORD IsHide			: 1;	//隐藏文件
			DWORD Leave				: 3;	//文件级别
			DWORD DosDrive			: 5;	//驱动器
			DWORD Index				: 22;	//文件索引
		};
		DWORD BasicInfo;
	};
};
*/


/*目录存储结构*/

/*
struct DIRECTORY_RECORD_NAME_LEN
{
	union{
		struct{
			WORD NameHasTwoByte : 1;	//最高位是否含有>0x7F
			WORD NameLen : 7;
		};
		WORD   NameLenInfo;
	};
};
*/
/*
typedef struct DIRECTORY_RECORD_INFO{
	//4位
	JC_DIRECTORY_BASIC_INFO BasicInfo;
	union{
		DIRECTORY_RECORD_INFO * pParentDirRecord;	   //快速索引时用到
		DWORD dwParentFileReferenceNumber;			   //初始化时用到
	};

	DIRECTORY_RECORD_NAME_LEN            NameLenInfo;

	//其后表示文件名长
	BYTE            Name[1];            //编码后的 文件名
}DIRECTORY_RECORD_INFO,*PDIRECTORY_RECORD_INFO;
*/


/*文件存储结构*/
typedef struct FILE_RECORD_INFO{


	bool operator==(const FILE_RECORD_INFO& l)
	{
		return BasicInfo.Index == l.BasicInfo.Index;
	}

	bool operator<(const FILE_RECORD_INFO& l)
	{
		return BasicInfo.Index < l.BasicInfo.Index;
	}

	JC_FILE_BASIC_INFO BasicInfo; 

	union{
		FILE_RECORD_INFO*	pParentDirRecord;				//父目录的指针
		DWORD	dwParentFileReferenceNumber;    //用于初始时建立目录树
	};

	BYTE            NameInformation[1]; //存储方式:文件名编码长度+文件名编码+[扩展名ID 若ext存在]
}FILE_RECORD_INFO, *PFILE_RECORD_INFO;


#pragma pack(pop)

struct mpool;

//编码文件
//返回编码后的长度
DWORD EncodFileName(LPCWSTR lpszFileName, const DWORD& dwNameLen, BYTE* szFileNameCode );
DWORD DecodeFileName(BYTE* szFileNameCode, const DWORD& dwCodeLen, LPWSTR wFileName);
DWORD DecodeFileNameNoCase(BYTE* szFileNameCode, const DWORD& dwCodeLen, LPWSTR wFileName);


DWORD GetFileName(PFILE_RECORD_INFO fileInfo, LPWSTR wFileName);
DWORD GetFileNameNoCase(PFILE_RECORD_INFO fileInfo, LPWSTR wFileName);

DWORD GetFileMemLenght(PFILE_RECORD_INFO fileInfo);

//获取扩展名长度所占用的字节数
DWORD GetExtendID(PFILE_RECORD_INFO fileInfo);
DWORD GetFileExtendIdMemLen(WORD dwID);

//获取文件长度所需要占用的字节数
DWORD GetFileNameLenMemLen(DWORD dwFileNameLen);

int GetFilePath( PFILE_RECORD_INFO, LPWSTR path);

inline DWORD GetResult(DWORD volume, DWORD file, DWORD leave, DWORD index)
{
	volume -= 'C';
	return QUERY_RESULT_INDEX(volume, file, leave, index).Result;
}

inline DWORD GetResultLeave(DWORD dwResult)
{
	return QUERY_RESULT_INDEX(dwResult).Leave;
}

inline CHAR GetResultVolume(DWORD dwResult)
{
	return QUERY_RESULT_INDEX(dwResult).Volume + 'C';
}

inline BOOL IsResultFile(DWORD dwResult)
{
	return QUERY_RESULT_INDEX(dwResult).IsFile;
}

inline BOOL GetResultIndex(DWORD dwResult)
{
	return QUERY_RESULT_INDEX(dwResult).Index;
}

static DWORD GetCoreVersions()
{
	return 4;
}

static DWORDLONG GetCurrentTime2()
{
	DWORDLONG dwCurrentTime = 0;

	SYSTEMTIME systemTime = { 0 };

	GetSystemTime(&systemTime);

	PWORD pTime = (PWORD)&dwCurrentTime;
	pTime[0] = systemTime.wYear;
	pTime[1] = systemTime.wMonth;
	pTime[2] = systemTime.wHour;
	pTime[3] = systemTime.wMinute;

	return dwCurrentTime;
}

WCHAR UpCase( WCHAR cch);

DWORD GetVolumeVolumeSerialNumber(WCHAR cDosName);

PFILE_RECORD_INFO CreateFileInfo(
	mpool*						pool,			//内存池
	const DWORD&			fileBasicInfo,	
	const DWORD&			parentInfo,
	LPCWSTR						lpszFileName,	//文件名称
	const DWORD&				dwFileLen,		//文件名长度
	const WORD&					pwExtensionId	//扩展名ID
	);


VOID DestroyFileInfo(mpool*	pool, PFILE_RECORD_INFO info);