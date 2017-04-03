#pragma once
#include<Windows.h>

//Unicode 转为 ANSI 编码
int UnicodeToANSI(char* out,int outsize, const TCHAR* str);
//ANSI 转为 Unicode 编码
int ANSItoUnicode(TCHAR* out, int outsize, const char* str);