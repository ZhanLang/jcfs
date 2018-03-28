
#include "stdafx.h"
#include "fSearchDef.h"
#include "mpool/mpool.h"
#include "libcrc/crc.h"

WCHAR UpCase( WCHAR cch)
{
	return cch >= L'a' ? cch - 32 : cch;
}


DWORD GetVolumeVolumeSerialNumber(WCHAR cDosName)
{
	WCHAR szVolumePath[8];
	swprintf_s(szVolumePath, _countof(szVolumePath), L"%c:\\", cDosName);

	WCHAR szVolumeMap[ 1024 ] = { 0 };
	if ( !GetVolumeNameForVolumeMountPoint(szVolumePath, szVolumeMap, _countof(szVolumeMap)) )
	{
		return 0;
	}

	CString strFilePath = szVolumeMap;
	strFilePath.MakeUpper();

	SHORT h = std_crc8((char*)strFilePath.GetBuffer(), (strFilePath.GetLength())* sizeof(TCHAR));
	INT   l = std_crc16((char*)strFilePath.GetBuffer(), (strFilePath.GetLength())* sizeof(TCHAR));

	return MAKELONG(l, h) ;	
}

DWORD EncodFileName(LPCWSTR lpszFileName, const DWORD& dwNameLen, BYTE* szFileNameCode)
{
	LPCWSTR pFileNameEnd = (LPCWSTR)lpszFileName + dwNameLen;
	LPBYTE pCodeBeg = szFileNameCode;
	LPBYTE pCodePtr = szFileNameCode;
	WCHAR wch = {0};

	for(;lpszFileName < pFileNameEnd ; ++lpszFileName)
	{
		wch = *lpszFileName;
		if( wch < 0x80 )
		{
			*pCodePtr++ = (BYTE)wch;
		}
		else
		{
			*pCodePtr++ = 0x81;
			*(PWCHAR)pCodePtr = wch;
			pCodePtr += 2;
		}
	}
	
	return  pCodePtr - pCodeBeg;
}

DWORD DecodeFileName(BYTE* szFileNameCode, const DWORD& dwCodeLen, LPWSTR wFileName)
{
	PBYTE pCode = szFileNameCode;
	PBYTE pCodeEnd = pCode + dwCodeLen;

	PWCHAR wFileName2 = wFileName;

	for( ; pCode < pCodeEnd ; ++wFileName2)
	{
		if(*pCode < 0x80)
		{
			*wFileName2 = *pCode++;
		}
		else
		{
			++pCode;
			*wFileName2 = *(PWCHAR)pCode;
			pCode += 2;
		}
	}

	*wFileName2 = L'\0';
	return  wFileName2 - wFileName;
}

DWORD DecodeFileNameNoCase(BYTE* szFileNameCode, const DWORD& dwCodeLen, LPWSTR wFileName)
{
	PBYTE pCode = szFileNameCode;
	PBYTE pCodeEnd = pCode + dwCodeLen;

	PWCHAR wFileName2 = wFileName;

	for( ; pCode < pCodeEnd ; ++wFileName2)
	{
		if(*pCode < 0x80)
		{
			*wFileName2 = g_NoCaseTable[*pCode++];
		}
		else
		{
			++pCode;
			*wFileName2 = *(PWCHAR)pCode;
			pCode += 2;
		}
	}

	*wFileName2 = L'\0';
	return  wFileName2 - wFileName;
}

DWORD GetFileName(PFILE_RECORD_INFO fileInfo, LPWSTR wFileName)
{
	if ( fileInfo->BasicInfo.TwoByteNameLen)
		return DecodeFileName(fileInfo->NameInformation + 2, *(WORD*)(fileInfo->NameInformation), wFileName);
	else
		return DecodeFileName(fileInfo->NameInformation + 1, *(fileInfo->NameInformation), wFileName);
	
	return 0;
}

DWORD GetFileNameNoCase(PFILE_RECORD_INFO fileInfo, LPWSTR wFileName)
{
	if ( fileInfo->BasicInfo.TwoByteNameLen)
		return DecodeFileNameNoCase(fileInfo->NameInformation + 2, *(WORD*)(fileInfo->NameInformation), wFileName);
	else
		return DecodeFileNameNoCase(fileInfo->NameInformation + 1, *(fileInfo->NameInformation), wFileName);

	return 0;
}




DWORD GetFileMemLenght(PFILE_RECORD_INFO fileInfo)
{
	//扩展名长度
	DWORD dwExtIdLength = fileInfo->BasicInfo.ExtensionLen;
	DWORD dwNameMemLen = 0;
	DWORD dwNameLen = 0;
	//四个字节
	if (3 == dwExtIdLength) 
		dwExtIdLength = 4;

	if ( fileInfo->BasicInfo.TwoByteNameLen )
	{
		dwNameMemLen = 2;
		dwNameLen = *(WORD*)(fileInfo->NameInformation);
	}
	else
	{
		dwNameMemLen = 1;
		dwNameLen = *(BYTE*)(fileInfo->NameInformation);
	}

	return sizeof(FILE_RECORD_INFO) + dwNameMemLen + dwNameLen +  dwExtIdLength - 1;
}

DWORD GetExtendID(PFILE_RECORD_INFO fileInfo)
{

	//没有扩展名
	if ( !fileInfo->BasicInfo.ExtensionLen )
		return 0;
	

	DWORD dwExtensionIdMemLen = fileInfo->BasicInfo.ExtensionLen;
	DWORD dwNameLenMemLen = 0;
	DWORD dwNameLen;


	if ( fileInfo->BasicInfo.TwoByteNameLen )
	{
		dwNameLenMemLen = 2;
		dwNameLen = *(WORD*)(fileInfo->NameInformation);
	}
	else
	{
		dwNameLenMemLen = 1;
		dwNameLen = *(BYTE*)(fileInfo->NameInformation);
	}

	BYTE* pNameEnd = fileInfo->NameInformation + dwNameLenMemLen + dwNameLen;
	if (dwExtensionIdMemLen > 0)
	{
		if (1 == dwExtensionIdMemLen) /*一个字节长度*/
		{
			return  *pNameEnd;
		}
		else
		{
			if (2 == dwExtensionIdMemLen)/*2字节长度*/
			{
				return *(WORD*)pNameEnd;;
			}
			else/*四个字节长度*/
			{
				return *(DWORD*)pNameEnd;
			}
		}
	}

	return 0;
}

DWORD GetFileExtendIdMemLen(WORD dwID)
{
	if ( dwID )
	{
		return dwID <0x100 ? 1 : ((UINT)dwID < 0x10000 ? 2 : 4);
	}

	return 0;
	
}

DWORD GetFileNameLenMemLen(DWORD dwFileNameLen)
{
	return dwFileNameLen < 0x100 ? 1 : 2;
}

int GetFilePath(PFILE_RECORD_INFO pFileRecord, LPWSTR lpszPath)
{
	WCHAR dirFileName[MAX_PATH] = { 0 };
	int nameLen;
	int i, j, iPath = 0;
	PFILE_RECORD_INFO pRecord = pFileRecord->pParentDirRecord;

	for (; pRecord != 0; pRecord = pRecord->pParentDirRecord)
	{
		lpszPath[iPath++] = L'\\';

		nameLen = GetFileName(pRecord, dirFileName);
		for (i = nameLen - 1; i >= 0; --i)
		{
			lpszPath[iPath++] = dirFileName[i];
		}
	}

	WCHAR wch;
	for (i = 0, j = iPath - 1; i < j; ++i, --j)
	{
		wch = lpszPath[i]; lpszPath[i] = lpszPath[j]; lpszPath[j] = wch;
	}

	lpszPath[iPath] = L'\0';
	return iPath;
}

PFILE_RECORD_INFO CreateFileInfo(
	mpool* pool, /*内存池 */ 
	const DWORD& ReferenceNumber, 
	const DWORD& dirReferenceNumber, 
	LPCWSTR lpszFileName, /*文件名称 */ 
	const DWORD& dwFileNameLen, /*文件名长度 */ 
	const WORD& pwExtensionId /*扩展名ID */
	)
{
	BYTE  szFileNameCode[768] = { 0 };//最多需要255*3个字节<768
	DWORD dwCodeFileNameLen = EncodFileName(lpszFileName, dwFileNameLen, szFileNameCode);

	
	DWORD dwCodeFileNameMemLen = GetFileNameLenMemLen(dwCodeFileNameLen);

	DWORD dwExtensionIdMemLen =  0 ;
	if ( pwExtensionId )
	{
		dwExtensionIdMemLen = GetFileExtendIdMemLen(pwExtensionId);
	}

	//总共需要的内存空间
	DWORD dwTotalMemLen = sizeof(FILE_RECORD_INFO) + dwCodeFileNameMemLen + dwCodeFileNameLen + dwExtensionIdMemLen - 1;

	PFILE_RECORD_INFO pFile = (PFILE_RECORD_INFO)mpool_salloc(pool, dwTotalMemLen);
	
	//PFILE_RECORD_INFO pFile = (PFILE_RECORD_INFO)malloc(dwTotalMemLen);
	ZeroMemory(pFile, dwTotalMemLen);
	pFile->BasicInfo.Index = ReferenceNumber;


	if ( dwCodeFileNameLen > dwFileNameLen )
		pFile->BasicInfo.NoNascii = TRUE;
	
	pFile->dwParentFileReferenceNumber = dirReferenceNumber;


	//设置文件名占用的字节数
	PBYTE pName = pFile->NameInformation, pNameEnd = NULL;
	if (2 == dwCodeFileNameMemLen)
	{
		pFile->BasicInfo.TwoByteNameLen = TRUE;
		*(WORD*)pName = (WORD)dwCodeFileNameLen;
		pName += 2;
	}
	else
	{
		*pName = (BYTE)dwCodeFileNameLen;
		++pName;
	}

	//设置编码后的文件名
	memcpy(pName, szFileNameCode, dwCodeFileNameLen);

	//设置扩展名
	pNameEnd = pName + dwCodeFileNameLen;
	if (dwExtensionIdMemLen > 0)
	{
		if (1 == dwExtensionIdMemLen) /*一个字节长度*/
		{
			pFile->BasicInfo.ExtensionLen  = 1;
			*pNameEnd = (BYTE)pwExtensionId;
		}
		else
		{
			if (2 == dwExtensionIdMemLen)/*2字节长度*/
			{
				pFile->BasicInfo.ExtensionLen = 2;
				*(WORD*)pNameEnd = (WORD)pwExtensionId;
			}
			else/*四个字节长度*/
			{
				pFile->BasicInfo.ExtensionLen = 3;
				*(DWORD*)pNameEnd = pwExtensionId;
			}
		}
	}

	return pFile;
}

VOID DestroyFileInfo(mpool* pool, PFILE_RECORD_INFO info)
{
	mpool_sfree(pool, info, GetFileMemLenght(info));
}
