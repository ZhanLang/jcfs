#pragma once
#include <ISsoft.h>
#include <thread/SubThreadImp.h>
#include <vector>
#include <syncobject/criticalsection.h>

typedef struct tag_soft
{
	CString strName;
	CString strLinkPath;
	CString strPinyin;
	CString strFrist;
} ST_SOFT;
typedef std::vector<ST_SOFT> CSoft;


class CSSoft : 
	public ISSoft,
	public CMsComBase<CSSoft>,
	public CSubThread
{
public:
	CSSoft();
	~CSSoft();
	UNKNOWN_IMP1( ISSoft);

protected:
	virtual DWORD	GetMatch( LPCWSTR strMatch , DWORD dwMatch = SOFT_MATCH_AUTO);
	virtual LPCWSTR GetLinkPath( DWORD dwIndex);
	virtual LPCWSTR GetName( DWORD dwIndex);
protected:
	virtual HRESULT	Run();

public:
	BOOL Build();
	BOOL Insert(ST_SOFT& soft);
private:
	

	CSoft m_soft;
	DECLARE_AUTOLOCK_CS(m_soft);

	CSoft m_result;
};

