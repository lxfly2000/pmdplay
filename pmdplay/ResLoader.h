#pragma once
#include<Windows.h>

//¼ÓÔØ×ÊÔ´×Ö·û´®
LPCWSTR LoadLocalStringW(UINT id);

#ifdef _UNICODE
#define LoadLocalString LoadLocalStringW
#endif
