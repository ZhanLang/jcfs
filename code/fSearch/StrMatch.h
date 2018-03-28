#pragma once

typedef const BYTE *LPCBYTE;

#define WCHAR_WCSSTR(/*INT const wchar_t **/ wcs1, /*INT const wchar_t **/ wcs2 , /*OUT wchar_t **/ ret)\
{\
	do \
	{\
		ret = 0;\
		wchar_t *cp = (wchar_t *)wcs1;\
		wchar_t *s1, *s2;\
		\
		if (!*wcs2)\
		{\
			ret = (int)wcs1;\
			break;\
		}\
		\
		while (*cp)\
		{\
			s1 = cp;\
			s2 = (wchar_t *)wcs2;\
			\
			while (*s1 && *s2 && !(*s1 - *s2))\
				s1++, s2++;\
				\
			if (!*s2)\
			{\
				ret = (int)(cp);\
				break;\
			}\
			\
			cp++;\
		}\
	} while (0);\
}\

#define BYTE_WCSSTR(/*INT const BYTE **/ wcs1, /*IN int */wcs1_len ,/*INT const BYTE **/ wcs2 , /*IN int */wcs2_len , /*OUT BOOL */ ret)\
{\
	do \
	{\
	ret = 0;\
	byte *cp = (byte *)wcs1;\
	byte *cp_end = cp + wcs1_len;\
	byte *wcs2_end = wcs2 + wcs2_len;\
	byte *s1, *s2;\
	\
	if (/*!*wcs2*/wcs2 == wcs2_end)\
		{\
		ret = (int)wcs1;\
		break;\
		}\
		\
		while (/**cp && */cp < cp_end)\
		{\
		s1 = cp;\
			s2 = (byte *)wcs2;\
		\
		while (s2 < wcs2_end && s1 < cp_end/**s1 && *s2*/ && !(*s1 - *s2))\
		s1++, s2++;\
		\
		if (/*!*s2*/s2 == wcs2_end)\
			{\
			ret = (int)(cp);\
			break;\
			}\
			\
			cp++;\
		}\
	} while (0);\
}\

#define BYTE_WCSSTR_NOCASE(/*INT const BYTE **/ wcs1, /*IN int */wcs1_len ,/*INT const BYTE **/ wcs2 , /*IN int */wcs2_len , /*OUT BYTE **/ ret)\
{\
	do \
	{\
	ret = 0;\
	byte *cp = (byte *)wcs1;\
	byte *cp_end = cp + wcs1_len;\
	byte *wcs2_end = wcs2 + wcs2_len;\
	byte *s1, *s2;\
	\
	if (/*!**/wcs2 == wcs2_end)\
		{\
		ret = (int)wcs1;\
		break;\
		}\
		\
		while (/**cp &&*/ cp < cp_end)\
		{\
		s1 = cp;\
		s2 = (byte *)wcs2;\
		\
		while (s2 < wcs2_end && s1 < cp_end /*&&*s1  && *s2 */ && !(g_NoCaseTable[*s1] - *s2))\
		s1++, s2++;\
		\
		if (/*!*s2*/s2 == wcs2_end)\
			{\
			ret = (int)(cp);\
			break;\
			}\
			\
			cp++;\
		}\
	} while (0);\
}\



inline wchar_t *  inline_wcsstr (const wchar_t * wcs1, const wchar_t * wcs2 )
{
	wchar_t *cp = (wchar_t *) wcs1;
	wchar_t *s1, *s2;

	if ( !*wcs2)
		return (wchar_t *)wcs1;

	while (*cp)
	{
		s1 = cp;
		s2 = (wchar_t *) wcs2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

inline wchar_t *  inline_wcscpy(wchar_t * dst, const wchar_t * src)
{
	wchar_t * cp = dst;

	while( *cp++ = *src++ );               /* Copy src over dst */

	return( dst );
}

inline wchar_t *  inline_wcsncpy(wchar_t * dst, const wchar_t * src, int len)
{
	wchar_t * cp = dst;
	
	while( (*cp++ = *src++) && --len);               /* Copy src over dst */
	*cp = L'\0';
	return( dst );
}

inline wchar_t * inline_wcswr(wchar_t * des, int nLen)
{
	wchar_t* des_begin = des;
	wchar_t* des_end = des + nLen;
   
	for ( ; des_begin < des_end ; ++des_begin)
	{
		if (L'A' <= *des_begin && *des_begin <= L'Z')
			*des_begin += (wchar_t)L'a' - (wchar_t)L'A';
	}

	return des;
}

inline int  inline_byte_strcmp (
	const byte * src,
	int src_len,

	const byte * dst,
	int dst_len
	)
{
	int ret = 0 ;
	const byte * src_end = src + src_len;
	const byte * dst_end = dst + dst_len;

	while(dst <= dst_end && dst <= dst_end && ! (ret = *(byte *)src - *(byte *)dst) )
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

#define WCHAR_WCSCMP(/*IN const wchar_t * */_src, /*IN const wchar_t **/ _dst , /*OUT int*/ ret)\
{\
	PCWCHAR src = _src; \
	PCWCHAR dst = _dst; \
	ret = 0;\
	while (!(ret = (int)(*src - *dst)) && *dst)\
		++src, ++dst;\
\
	if (ret == 0)\
		ret = 1;\
	else\
		ret = 0;\
}\



#define BYTE_WCSCMP(/*const byte **/ _src, /*int*/ wcs1_len ,  /*const byte * */_dst , /*IN int */wcs2_len, /*int&*/ ret)\
{\
	LPCBYTE src = _src; \
	LPCBYTE dst = _dst;\
	LPCBYTE src_end = _src + wcs1_len;\
	LPCBYTE dst_end = _dst + wcs2_len;\
	ret = 0;\
	while (src < src_end && dst < dst_end && !(ret = (int)(*src - *dst)) )\
		++src, ++dst;\
		\
	if (ret == 0)\
		ret = 1;\
	else\
		ret = 0;\
}\


static void INLINT_BYTE_WCSCMP(const byte * _src, int wcs1_len ,  const byte * _dst , IN int wcs2_len, int& ret)
{
	LPCBYTE src = _src; 
	LPCBYTE dst = _dst;
	LPCBYTE src_end = _src + wcs1_len;
	LPCBYTE dst_end = _dst + wcs2_len;
	ret = 0;
	while (src < src_end && dst < dst_end && !(ret = (byte)(*src - *dst)) )
	++src, ++dst;
	
	if (ret == 0)
	ret = 1;
	else
	ret = 0;
	if (ret == 1)
	{
		int a = 0;
	}
}

#define   BYTE_WCSCMP_NO_CASE(/*const byte **/ _src, /*int*/ wcs1_len ,  /*const byte * */_dst , /*IN int */wcs2_len, /*int&*/ ret)\
{\
	LPCBYTE src = _src; \
	LPCBYTE dst = _dst;\
	LPCBYTE src_end = _src + wcs1_len;\
	LPCBYTE dst_end = _dst + wcs2_len;\
	ret = 0;\
	while (src < src_end && dst < dst_end && !(ret = (byte)(g_NoCaseTable[*src] - *dst)) )\
		++src, ++dst;\
		\
	if (ret == 0)\
		ret = 1;\
	else\
		ret = 0;\
}\


/*
static void INLINT_BYTE_WCSCMP_NO_CASE(const byte * _src, int wcs1_len ,  const byte * _dst , IN int wcs2_len, int& ret)
{
	LPCBYTE src = _src; 
	LPCBYTE dst = _dst;
	LPCBYTE src_end = _src + wcs1_len;
	LPCBYTE dst_end = _dst + wcs2_len;
	ret = 0;
	while (src < src_end && dst < dst_end && !(ret = (byte)(g_NoCaseTable[*src] - *dst)) )
		++src, ++dst;

	if (ret == 0)
		ret = 1;
	else
		ret = 0;
}
*/


inline int  inline_wcscmp (const wchar_t * src,const wchar_t * dst)
{
	int ret = 0 ;

	while( ! (ret = (int)(*src - *dst)) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

inline wchar_t * inline_wcscat ( wchar_t * dst, const wchar_t * src )
{
	wchar_t * cp = dst;

	while( *cp )
		cp++;                   /* find end of dst */

	while( *cp++ = *src++ ) ;       /* Copy src to end of dst */

	return( dst );                  /* return dst */

}

inline  size_t  inline_wcslen (const wchar_t * wcs)
{
	const wchar_t *eos = wcs;

	while( *eos++ ) ;

	return( (size_t)(eos - wcs - 1) );
}

template<typename T>
static inline void insertion_sort(T* unsorted, INT nLen)
{
	for (int i = 1; i < nLen; i++)
	{
		if (unsorted[i - 1] > unsorted[i])
		{
			T temp = unsorted[i];
			int j = i;
			while (j > 0 && unsorted[j - 1] > temp)
			{
				unsorted[j] = unsorted[j - 1];
				j--;
			}
			unsorted[j] = temp;
		}
	}
}


/*二分查找算法*/
template<typename T>
static inline int binary_search(T *array, const int size, const T & key)
{
	if(NULL == array || size < 1)
	{
		return -1;
	}

	int low = 0, high = size - 1;
	int mid_index = 0;

	while(low <= high)
	{
		mid_index = (low+high)/2;

		if(key == array[mid_index])
		{
			return mid_index;
		}  
		else if(key > array[mid_index])
		{
			low = mid_index + 1; 
		}
		else 
		{
			high = mid_index - 1;
		}
	}

	return  -1;
}

static inline void insertion_sort_with_strlen(LPCWSTR* unsorted, INT nLen)
{
	for (int i = 1; i < nLen; i++)
	{
		if (inline_wcslen(unsorted[i - 1]) < inline_wcslen(unsorted[i]))
		{
			LPCWSTR temp = unsorted[i];
			int j = i;
			while (j > 0 && inline_wcslen(unsorted[j - 1]) < inline_wcslen(temp))
			{
				unsorted[j] = unsorted[j - 1];
				j--;
			}
			unsorted[j] = temp;
		}
	}
}


//获取按字符串长度一次变短的字串
static BOOL GetSortSubStr(LPTSTR lpszSrc, LPCTSTR* nBuffer, LPCTSTR lpszDelimit, INT& nCount)
{
	TCHAR *next_token = NULL;
	nCount = 0;

	TCHAR *szCmd = _tcstok_s(lpszSrc, lpszDelimit, &next_token);
	while( szCmd != NULL && _tcslen(szCmd))
	{
		nBuffer[nCount] = szCmd;
		nCount++;
		szCmd = _tcstok_s( NULL, lpszDelimit, &next_token );
	}

	if (nCount)
	{
		insertion_sort_with_strlen(nBuffer,nCount);
	}

	return TRUE;
}