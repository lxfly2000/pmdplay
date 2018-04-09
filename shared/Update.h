#pragma once
#include<Windows.h>
static const TCHAR projectURL[] = TEXT("https://github.com/lxfly2000/pmdplay");
static const TCHAR updateFileURL[] = TEXT("https://github.com/lxfly2000/pmdplay/raw/master/shared/resource.h");
//检查更新，出错返回负数，无更新返回0，有更新返回1
int CheckForUpdate(const TCHAR *fileURL, int *newVersion);
