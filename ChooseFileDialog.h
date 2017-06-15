#pragma once
#include<Windows.h>

//打开对话框，选择一个文件，所给参数必须要有初始化，filepath 参数可以预先给一个路径作为默认位置
BOOL ChooseFile(HWND hWndParent, TCHAR *filepath, TCHAR *filename, PCTSTR pcszFiletype, PCTSTR pcszTitle);
//打开保存对话框，指定一个文件，所给参数必须要有初始化，filepath 参数可以预先给一个路径作为默认位置
BOOL ChooseSaveFile(HWND hWndParent, TCHAR *filepath, TCHAR *filename, PCTSTR pcszFiletype, PCTSTR pcszTitle);
//打开对话框，选择一个目录，需要调用 CoInitialize 和 CoUninitialize 初始化/释放 COM 组件
BOOL ChooseDirectory(HWND hWndParent, TCHAR *fullPath, PCTSTR pcszDefaultPath, PCTSTR pcszTitle);
//打开 Win9X 风格对话框，选择一个目录
BOOL ChooseDirectoryClassic(HWND hWndParent, TCHAR *fullPath, PCTSTR pcszDefaultPath, PCTSTR pcszInstruction);
