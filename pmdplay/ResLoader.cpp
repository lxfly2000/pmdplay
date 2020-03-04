#include "ResLoader.h"

static WCHAR resBuffer[4096];

LPCWSTR LoadLocalStringW(UINT id)
{
	int copiedCharLength = LoadStringW(GetModuleHandleW(NULL), id, resBuffer, ARRAYSIZE(resBuffer));
	return copiedCharLength ? resBuffer : nullptr;
}
