#pragma once
#include<Windows.h>
//检查更新，出错返回负数，无更新返回0，有更新返回1
int CheckForUpdate(const TCHAR *fileURL, int *newVersion);
