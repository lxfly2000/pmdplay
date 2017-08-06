#pragma once
#include<Windows.h>
//显示输入对话框，按确定返回 1, 取消返回 0;
//指定 textExtraButton 时会显示第三个按钮并以指定文字显示，按下该按钮后返回 2;
//maxlen 为 -1 时表示不管 pszOut 的长度，得到的字符串最大长度为 maxlen - 1;
//无论按哪个按钮 pszOut 都会返回输入的字符串。
int InputBox(HWND hwndParent, PTSTR pszOut, int maxlen, PCTSTR msgText, PCTSTR title, PCTSTR defaultInput = NULL,
	PCTSTR textExtraButton = NULL, PCTSTR textOK = NULL, PCTSTR textCancel = NULL);