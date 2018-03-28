#pragma once
#include <helper/SAdapterBase.h>
#include <IfSearch.h>
#include <combase/IMsVector.h>
class CFsMcAdapterFix :
	public SMcAdapterBase
{
public:
	
	IDiskSearchCli* m_pDiskSearch;
	DWORD m_dwResultCount;
	CFsMcAdapterFix( IDiskSearchCli* pDiskSearch, DWORD  dwResultCount)
	{
		m_pDiskSearch = pDiskSearch;
		m_dwResultCount = dwResultCount;
	}

	virtual int getCount()
	{
		return m_dwResultCount;

		return 0;
	}   

	SStringT getSizeText(DWORD dwSize)
	{
		int num1=dwSize/(1<<20);
		dwSize -= num1 *(1<<20);
		int num2 = dwSize*100/(1<<20);
		return SStringT().Format(_T("%d.%02dM"),num1,num2);
	}

	virtual void getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0)
			pItem->InitFromXml(xmlTemplate);
		
		if ( !( m_dwResultCount && m_pDiskSearch ) )
			return ;
		

		if ( position > m_dwResultCount)
			return ;
		




		DWORD dwGetValueMask = 
			GetValueMask_IcoIndex		| //获取图标索引
			GetValueMask_FileSize		| //获取文件大小
			GetValueMask_MondifyTime	| //获取修改时间
			GetValueMask_CreateTime		| //获取创建时间
			GetValueMask_AccessTime		| //获取访问时间
			GetValueMask_Attributes		; //获取属性

		SearchResult Result = { 0 };

		
		if ( FAILED(m_pDiskSearch->GetResultAt(position, dwGetValueMask, Result)) )
		{
			return ;
		}


		SHFILEINFO    shfi; //文件信息结构变量用于存放函数调用的结果
		memset(&shfi,0,sizeof(shfi));

		SStringT strFilePath;
		strFilePath.Format(_T("%s\\%s"), Result.szPath, Result.szPath);

		//SHGetFileInfo(strFilePath, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),SHGFI_ICON);

		SIconWnd* pFileIcon = pItem->FindChildByName2<SIconWnd>(L"file_ico");
		SStatic*  pFileName = pItem->FindChildByName2<SStatic>(L"file_name");
		if ( pFileName ) pFileName->SetWindowText( Result.szName);

// 		SOFTINFO *psi =m_softInfo.GetData()+position%m_softInfo.GetCount();
// 		pItem->FindChildByName(L"txt_name")->SetWindowText(S_CW2T(psi->pszName));
// 		pItem->FindChildByName(L"txt_installtime")->SetWindowText(getSizeText(psi->dwSize));
// 		pItem->FindChildByName(L"txt_size")->SetWindowText(S_CW2T(psi->pszDesc));
// 		SButton *pBtnUninstall = pItem->FindChildByName2<SButton>(L"btn_uninstall");
// 		pBtnUninstall->SetUserData(position);
		//pBtnUninstall->GetEventSet()->subscribeEvent(EVT_CMD,Subscriber(&CFsMcAdapterFix::OnButtonClick,this));
	}

	bool OnButtonClick(EventArgs *pEvt)
	{
		SButton *pBtn = sobj_cast<SButton>(pEvt->sender);
		int iItem = pBtn->GetUserData();

		if(SMessageBox(NULL,SStringT().Format(_T("Are you sure to uninstall the selected [%d] software?"),iItem),_T("uninstall"),MB_OKCANCEL|MB_ICONQUESTION)==IDOK)
		{//删除一条记录
			DeleteItem(iItem);
		}
		return true;
	}

	//删除一行，提供外部调用。
	void DeleteItem(int iPosition)
	{
		
	}

	SStringW GetColumnName(int iCol) const{
		return SStringW().Format(L"col%d",iCol+1);
	}

	struct SORTCTX
	{
		int iCol;
		SHDSORTFLAG stFlag;
	};

	bool OnSort(int iCol,SHDSORTFLAG * stFlags,int nCols)
	{
		if(iCol==5) //最后一列“操作”不支持排序
			return false;

// 		SHDSORTFLAG stFlag = stFlags[iCol];
// 		switch(stFlag)
// 		{
// 		case ST_NULL:stFlag = ST_UP;break;
// 		case ST_DOWN:stFlag = ST_UP;break;
// 		case ST_UP:stFlag = ST_DOWN;break;
// 		}
// 		for(int i=0;i<nCols;i++)
// 		{
// 			stFlags[i]=ST_NULL;
// 		}
// 		stFlags[iCol]=stFlag;
// 
// 		SORTCTX ctx={iCol,stFlag};
// 		qsort_s(m_softInfo.GetData(),m_softInfo.GetCount(),sizeof(SOFTINFO),SortCmp,&ctx);
		return true;
	}

	static int __cdecl SortCmp(void *context,const void * p1,const void * p2)
	{
		return 0;
		/*SORTCTX *pctx = (SORTCTX*)context;
		const SOFTINFO *pSI1=(const SOFTINFO*)p1;
		const SOFTINFO *pSI2=(const SOFTINFO*)p2;
		int nRet =0;
		switch(pctx->iCol)
		{
		case 0://name
			nRet = wcscmp(pSI1->pszName,pSI2->pszName);
			break;
		case 1://score
			{
				float fCmp = (pSI1->fScore - pSI2->fScore);
				if(fabs(fCmp)<0.0000001) nRet = 0;
				else if(fCmp>0.0f) nRet = 1;
				else nRet = -1;
			}
			break;
		case 2://size
			nRet = (int)(pSI1->dwSize - pSI2->dwSize);
			break;
		case 3://install time
			nRet = wcscmp(pSI1->pszInstallTime,pSI2->pszInstallTime);
			break;
		case 4://user time
			nRet = wcscmp(pSI1->pszUseTime,pSI2->pszUseTime);
			break;

		}
		if(pctx->stFlag == ST_UP)
			nRet = -nRet;
		return nRet;
		*/
	}
};

