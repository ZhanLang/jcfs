#include "stdafx.h"
#include "UIAnimatRotate.h"


DECLARE_HANDLE(HZIP);	// An HZIP identifies a zip file that has been opened
typedef DWORD ZRESULT;
typedef struct
{ 
	int index;                 // index of this file within the zip
	char name[MAX_PATH];       // filename within the zip
	DWORD attr;                // attributes, as in GetFileAttributes.
	FILETIME atime,ctime,mtime;// access, create, modify filetimes
	long comp_size;            // sizes of item, compressed and uncompressed. These
	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
} ZIPENTRY;
typedef struct
{ 
	int index;                 // index of this file within the zip
	TCHAR name[MAX_PATH];      // filename within the zip
	DWORD attr;                // attributes, as in GetFileAttributes.
	FILETIME atime,ctime,mtime;// access, create, modify filetimes
	long comp_size;            // sizes of item, compressed and uncompressed. These
	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
} ZIPENTRYW;
#define OpenZip OpenZipU
#define CloseZip(hz) CloseZipU(hz)
extern HZIP OpenZipU(void *z,unsigned int len,DWORD flags);
extern ZRESULT CloseZipU(HZIP hz);
#ifdef _UNICODE
#define ZIPENTRY ZIPENTRYW
#define GetZipItem GetZipItemW
#define FindZipItem FindZipItemW
#else
#define GetZipItem GetZipItemA
#define FindZipItem FindZipItemA
#endif
extern ZRESULT GetZipItemA(HZIP hz, int index, ZIPENTRY *ze);
extern ZRESULT GetZipItemW(HZIP hz, int index, ZIPENTRYW *ze);
extern ZRESULT FindZipItemA(HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRY *ze);
extern ZRESULT FindZipItemW(HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRYW *ze);
extern ZRESULT UnzipItem(HZIP hz, int index, void *dst, unsigned int len, DWORD flags);

CAnimatRotate::CAnimatRotate()
{
	m_nStep = 0;
	m_bitmap = NULL;
}

CAnimatRotate::~CAnimatRotate()
{
	if (m_bitmap){
		delete m_bitmap;
	}
}

void CAnimatRotate::PaintBkImage(HDC hDC)
{
	if( m_sBkImage.IsEmpty() ) return;

	int x = (m_rcItem.right - m_rcItem.left)/2 + m_rcItem.left;
	int y = (m_rcItem.bottom - m_rcItem.top)/2 + m_rcItem.top;

	Gdiplus::Graphics graphics(hDC);

	int nTmp = 10*m_nStep;
	if (nTmp % 360 == 0){
		m_nStep = 0;
	}
	//nTmp = 0;
	Gdiplus::REAL rl = (Gdiplus::REAL)nTmp;
	graphics.TranslateTransform((Gdiplus::REAL)x, (Gdiplus::REAL)y);
	graphics.RotateTransform(rl);

	Gdiplus::Point pt(-(m_rcItem.right - m_rcItem.left)/2, -(m_rcItem.bottom - m_rcItem.top)/2);
	
	//Gdiplus::Point P(x, y);
	graphics.DrawImage(m_bitmap, pt);
	//graphics.ResetTransform();

}

static Gdiplus::Bitmap* LoadPngFromMemory( LPVOID pBuf, size_t dwSize )  
{  
	HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);  
	BYTE* pMem = (BYTE*)::GlobalLock(hMem);  

	memcpy(pMem, pBuf, dwSize);  

	IStream* pStm = NULL;  
	::CreateStreamOnHGlobal(hMem, TRUE, &pStm);  
	Gdiplus::Bitmap *pImg = Gdiplus::Bitmap::FromStream(pStm);  
	if(!pImg || pImg->GetLastStatus() != Gdiplus::Ok)  
	{  
		pStm->Release();  
		::GlobalUnlock(hMem);  
		return 0;  
	}  
	return pImg;  
}  

void CAnimatRotate::DoInit()
{
	m_pManager->SetTimer(this, 0xFF, 50);
	
	CDuiString pstrGifPath = m_sBkImage;

	LPBYTE pData = NULL;
	DWORD dwSize = 0;

	do 
	{
		CDuiString sFile = CPaintManagerUI::GetResourcePath();
		if( CPaintManagerUI::GetResourceZip().IsEmpty() ) {
			sFile += pstrGifPath;
			HANDLE hFile = ::CreateFile(sFile.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
				FILE_ATTRIBUTE_NORMAL, NULL);
			if( hFile == INVALID_HANDLE_VALUE ) break;
			dwSize = ::GetFileSize(hFile, NULL);
			if( dwSize == 0 ) break;

			DWORD dwRead = 0;
			pData = new BYTE[ dwSize ];
			::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
			::CloseHandle( hFile );

			if( dwRead != dwSize ) {
				delete[] pData;
				pData = NULL;
				break;
			}
		}
		else {
			sFile += CPaintManagerUI::GetResourceZip();
			HZIP hz = NULL;
			if( CPaintManagerUI::IsCachedResourceZip() ) hz = (HZIP)CPaintManagerUI::GetResourceZipHandle();
			else hz = OpenZip((void*)sFile.GetData(), 0, 2);
			if( hz == NULL ) break;
			ZIPENTRY ze; 
			int i; 
			if( FindZipItem(hz, pstrGifPath, true, &i, &ze) != 0 ) break;
			dwSize = ze.unc_size;
			if( dwSize == 0 ) break;
			pData = new BYTE[ dwSize ];
			int res = UnzipItem(hz, i, pData, dwSize, 3);
			if( res != 0x00000000 && res != 0x00000600) {
				delete[] pData;
				pData = NULL;
				if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
				break;
			}
			if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
		}

	} while (0);

	while (!pData)
	{
		//读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
		HANDLE hFile = ::CreateFile(pstrGifPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
			FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) break;
		dwSize = ::GetFileSize(hFile, NULL);
		if( dwSize == 0 ) break;

		DWORD dwRead = 0;
		pData = new BYTE[ dwSize ];
		::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
		::CloseHandle( hFile );

		if( dwRead != dwSize ) {
			delete[] pData;
			pData = NULL;
		}
		break;
	}
	if (!pData)
	{
		return;
	}
	m_bitmap = LoadPngFromMemory((LPVOID)pData, dwSize);
	
	delete[] pData;
	pData = NULL;
	return ;
}

void CAnimatRotate::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_TIMER && event.wParam == 0xFF)
	{
		m_nStep++;
		NeedParentUpdate();
	}
}

void CAnimatRotate::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	CLabelUI::SetAttribute(pstrName, pstrValue);
}
