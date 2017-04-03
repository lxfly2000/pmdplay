#include"TextEncodeConvert.h"

//http://blog.chinaunix.net/uid-23860671-id-189905.html
int UnicodeToANSI(char* out,int outsize, const TCHAR* str)
{
	int needlength = WideCharToMultiByte(CP_ACP, NULL, str, -1, NULL, 0, NULL, FALSE);
	if (outsize < needlength)return FALSE;
	return WideCharToMultiByte(CP_ACP, NULL, str, -1, out, outsize, NULL, FALSE);
}
int ANSItoUnicode(TCHAR* out, int outsize, const char* str)
{
	int needlength = MultiByteToWideChar(CP_ACP, NULL, str, -1, NULL, 0);
	if (outsize < needlength)return FALSE;
	return MultiByteToWideChar(CP_ACP, NULL, str, -1, out, needlength);
}