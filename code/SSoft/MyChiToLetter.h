//实现汉字向拼音的转化
//-----------------------------------------------------设计人:牛文平
class CChiToLetter
{
public:
	CChiToLetter();
	~CChiToLetter();
	//分隔符
	BOOL m_LetterEnd;
	//TRUE:得到首字母大写
	//FALSE:得到首字母小写
	BOOL m_blnFirstBig;
	//TRUE:得到全部大写
	//FALSE:得到除去首拼音的所有小写
	BOOL m_blnAllBiG;
	//True:得到全部拼音
	//FALSE:得到首拼音
	BOOL m_blnSimaple;
	//返回拼音
	CString GetLetter(CStringA strText);
private:
	CString FindLetter(int nCode);
};