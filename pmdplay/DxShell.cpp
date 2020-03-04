#pragma once
#include"DxShell.h"
#include"DxKeyTrigger.h"
#include"ResLoader.h"
#include"resource1.h"
#include<Shlwapi.h>
#include<deque>
#pragma comment(lib,"shlwapi.lib")
//缩短路径以使字符串的绘制宽度小于maxWidth
//返回值为shortend
TCHAR *ShortenPath(const TCHAR *src, BOOL isDir, TCHAR *shortened, int fontHandle, int maxWidth)
{
	int w = GetDrawStringWidthToHandle(src, (int)strlenDx(src), fontHandle);
	if (w <= maxWidth)return (TCHAR*)src;
	int pc = strrchr2Dx(src, '\\');
	TCHAR fileext[MAX_PATH] = TEXT("");
	if (pc == -1)strncpyDx(fileext, src, ARRAYSIZE(fileext));
	else strncpyDx(fileext, src + pc + 1, ARRAYSIZE(fileext));
	pc = strrchr2Dx(fileext, '.');
	if (pc == -1||isDir)fileext[0] = 0;
	else
	{
		strcpyDx(fileext, fileext + pc + 1);
		TCHAR testOnlyExt[MAX_PATH];
		sprintfDx(testOnlyExt, TEXT("...%s"), fileext);
		if (GetDrawStringWidthToHandle(testOnlyExt, (int)strlenDx(testOnlyExt), fontHandle) > maxWidth)
			fileext[0] = 0;
	}
	strcpyDx(shortened, src);
	while (w > maxWidth)
	{
		pc = strchr2Dx(shortened, '\\');
		if (pc != -1)
		{
			if (strchr2Dx(shortened + pc + 1, '\\') != -1)
			{
				shortened[strchr2Dx(shortened, '\\')] = '_';
				strcpyDx(shortened, shortened + strchr2Dx(shortened, '\\') - 3);
				for (int i = 0; i < 3; i++)shortened[i] = '.';
			}
			else goto tagShortenExt;
		}
		else
		{
		tagShortenExt:
			sprintfDx(shortened + strlenDx(shortened) - strlenDx(fileext) - 4, TEXT("...%s"), fileext);
		}
		w = GetDrawStringWidthToHandle(shortened, (int)strlenDx(shortened), fontHandle);
	}
	return shortened;
}
//返回src盘符的下一个盘符，返回值为next
TCHAR *GetNextLogicalDriveString(const TCHAR *src, TCHAR *next)
{
	TCHAR firstd = 0, itdc = 'A', cdc = *src;
	DWORD drives = GetLogicalDrives();
	bool ok = false;
	for (int i = 0; i < 26; i++)
	{
		if (drives & 1)
		{
			if (!firstd)firstd = itdc;
			if (itdc > cdc)
			{
				*next = itdc;
				ok = true;
				break;
			}
		}
		drives >>= 1;
		itdc++;
	}
	if (!ok)*next = firstd;
	next[1] = ':';
	next[2] = '\\';
	next[3] = 0;
	return next;
}
//获取目录中文件和文件夹项目，如果pdata为NULL则只返回总数
int GetItemsInPath(const TCHAR *path, WIN32_FIND_DATA *pdata)
{
	TCHAR checkpath[MAX_PATH] = TEXT("");
	PathCombine(checkpath, path, TEXT("*"));
	WIN32_FIND_DATA td;
	HANDLE hf = FindFirstFile(checkpath, pdata ? pdata : &td);
	if (hf == INVALID_HANDLE_VALUE)return 0;
	int c = 1;
	while (FindNextFile(hf, pdata ? pdata + c : &td))c++;
	FindClose(hf);
	return c;
}
static bool useHighDpi = true;
void DxShellSetUseHighDpi(bool b)
{
	useHighDpi = b;
}
template<typename Tnum>Tnum HdpiNum(Tnum n)
{
	return useHighDpi ? (Tnum)(GetDeviceCaps(GetDC(NULL), LOGPIXELSX) * n / USER_DEFAULT_SCREEN_DPI) : n;
}
int DxShellCreateFontToHandle(const TCHAR *fontname, int fontsize, int fontthick)
{
	if (fontthick == DXGUI_FONTTHICK_DEFAULT)
		fontthick = 3;
	if (fontsize == DXGUI_FONTSIZE_DEFAULT)
		fontsize = HdpiNum(14);
	return CreateFontToHandle(fontname, fontsize, fontthick, fontsize > 14 ? DX_FONTTYPE_ANTIALIASING : -1);
}
//cx,cy有虚值DXGUI_POSITION_CENTER表示屏幕中心
int DxMessageBox(const TCHAR *msg, int keyOk, int keyCancel, int strcolor, int bgcolor, int bordercolor, float borderwidth, const TCHAR *fontname,
	int fontsize, int fontthick, int cx, int cy, int paddingWidth, int paddingHeight)
{
	int hDxFont = DxShellCreateFontToHandle(fontname, fontsize, fontthick);
	if (hDxFont == -1)return FALSE;
	int ww, wh;
	GetDrawScreenSize(&ww, &wh);
	if (cx == DXGUI_POSITION_CENTER)cx = ww / 2;
	if (cy == DXGUI_POSITION_CENTER)cy = wh / 2;

	//drawstring msg
	int strw, strh, lc;
	GetDrawStringSizeToHandle(&strw, &strh, &lc, msg, (int)strlenDx(msg), hDxFont);
	if (paddingWidth == -1)paddingWidth = (ww - strw) / 2;
	if (paddingHeight == -1)paddingHeight = (wh - strh) / 2;
	cx = cx - strw / 2 - paddingWidth;
	cy = cy - strh / 2 - paddingHeight;
	DrawBox(cx, cy, cx + strw + 2 * paddingWidth, cy + strh + 2 * paddingHeight, bgcolor, TRUE);
	if (borderwidth > 0.0f)
		DrawBoxAA((float)cx, (float)cy, cx + strw + 2.0f * paddingWidth, cy + strh + 2.0f * paddingHeight, bordercolor, FALSE, borderwidth);
	DrawStringToHandle(cx + paddingWidth, cy + paddingHeight, msg, strcolor, hDxFont);
	ScreenFlip();
	int ret = -1;
	DxKeyTrigger trKeyOk(keyOk), trKeyCancel(keyCancel);
	do {
		if (trKeyOk.Released())
			ret = TRUE;
		if (trKeyCancel.Released())
			ret = FALSE;
	} while (ret == -1);
	DeleteFontToHandle(hDxFont);
	return ret;
}
const TCHAR *ParseFileNameFromPath(const TCHAR *path, TCHAR *filename)
{
	for (int i = (int)strlenDx(path) - 1; i >= 0; i--)
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			strcpyDx(filename, path + i + 1);
			return filename;
		}
	}
	return NULL;
}
//cx,cy有虚值DXGUI_POSITION_CENTER表示屏幕中心
int DxChooseFilePath(const TCHAR *initPath, TCHAR *choosedPath, const TCHAR *msg, int chooseDir, int keyOk, int keyCancel, int strcolor,
	int bgcolor, int bordercolor, float borderwidth, const TCHAR *fontname, int fontsize, int fontthick, int cx, int cy, int paddingWidth,
	int paddingHeight)
{
	TCHAR msg_def[50];
	strcpyDx(msg_def, LoadLocalString(IDS_DXGUI_CHOOSEFILE_MSG_DEFAULT));
	if (msg == nullptr)
		msg = msg_def;
	int hDxFont = DxShellCreateFontToHandle(fontname, fontsize, fontthick);
	if (hDxFont == -1)return FALSE;
	int ww, wh;
	GetDrawScreenSize(&ww, &wh);
	if (cx == DXGUI_POSITION_CENTER)cx = ww / 2;
	if (cy == DXGUI_POSITION_CENTER)cy = wh / 2;
	
	const int list_show_items = 5;
	int strw, strh, lc, singlelineh;
	GetDrawStringSizeToHandle(&strw, &singlelineh, &lc, TEXT("高"), 1, hDxFont);//获取一行字的高度，也可以用singlelineh = strh / lc获取
	GetDrawStringSizeToHandle(&strw, &strh, &lc, msg, (int)strlenDx(msg), hDxFont);
	strw = max(strw, HdpiNum(DXGUI_DRAWSTRING_MIN_WIDTH));
	if (paddingWidth == -1)paddingWidth = (ww - strw) / 2;
	int listy = strh;
	strh += (list_show_items + 1) * singlelineh;
	if (paddingHeight == -1)paddingHeight = (wh - strh) / 2;
	cx = cx - strw / 2 - paddingWidth;
	cy = cy - strh / 2 - paddingHeight;
	int listx = cx + paddingWidth;
	listy += cy + paddingHeight;

	int keypressed;
	std::deque<int> keyqueue;
	int ret = FALSE;
	int ci;
	WIN32_FIND_DATA *fd = nullptr;
	int cur, listpagecur;
	TCHAR tempPath[MAX_PATH] = TEXT(""), shortenedPath[MAX_PATH] = TEXT("");
	ConvertFullPath(initPath, tempPath);
	TCHAR findingInitPath[MAX_PATH] = TEXT("");
	if (!PathIsDirectory(tempPath))
		ParseFileNameFromPath(tempPath, findingInitPath);
	while (!(PathFileExists(tempPath) && PathIsDirectory(tempPath)))
	{
		if (!PathFileExists(tempPath))
			findingInitPath[0] = 0;
		PathCombine(tempPath, tempPath, TEXT(".."));
		if (PathIsRoot(tempPath))
			break;
	}
tagUpdateDir:
	listpagecur = 0;
	cur = 0;
	ci = GetItemsInPath(tempPath, NULL);
	if (ci == 0)
	{
		findingInitPath[0] = 0;
		DxMessageBox(LoadLocalString(IDS_DXMSG_EMPTY_FOLDER));
		if (strlenDx(tempPath) < 4)GetNextLogicalDriveString(tempPath, tempPath);
		else PathCombine(tempPath, tempPath, TEXT(".."));
		goto tagUpdateDir;
	}
	if (fd)delete fd;
	fd = new WIN32_FIND_DATA[ci];
	ci = GetItemsInPath(tempPath, fd);
	if (findingInitPath[0])
	{
		listpagecur = max(0, ci - list_show_items);
		cur = ci - listpagecur - 1;
		while (listpagecur + cur > 0)
		{
			if (strcmpDx(findingInitPath, fd[listpagecur + cur].cFileName) == 0)
				break;
			if (cur > 0)
				cur--;
			else
				listpagecur--;
		}
		findingInitPath[0] = 0;
	}
tagCursorMove:
	DrawBox(cx, cy, cx + strw + 2 * paddingWidth, cy + strh + 2 * paddingHeight, bgcolor, TRUE);
	if (borderwidth > 0.0f)
		DrawBoxAA((float)cx, (float)cy, cx + strw + 2.0f * paddingWidth, cy + strh + 2.0f * paddingHeight, bordercolor, FALSE, borderwidth);
	DrawStringToHandle(listx, cy + paddingHeight, msg, strcolor, hDxFont);

	for (int i = 0; i < list_show_items; i++)
	{
		if (listpagecur + i >= ci)break;
		if (i == cur)
			DrawBox(listx, listy + singlelineh*i, listx + strw, listy + singlelineh*(i + 1), strcolor,
				!(fd[listpagecur + i].dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY));
		DrawStringToHandle(listx, listy + singlelineh*i, ShortenPath(fd[listpagecur + i].cFileName,
			fd[listpagecur+i].dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY,shortenedPath,hDxFont,strw),
			((i == cur) && !(fd[listpagecur + i].dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) ? bgcolor : strcolor, hDxFont);
	}
	DrawStringToHandle(listx, listy + singlelineh * list_show_items, ShortenPath(tempPath,TRUE,shortenedPath,hDxFont,strw), strcolor, hDxFont);
	
	ScreenFlip();
	if (keyqueue.empty())
		keyqueue.push_back(WaitKey());
	keypressed = keyqueue.front();
	keyqueue.pop_front();
	if (keypressed == keyOk)
	{
		if (fd[listpagecur + cur].dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (chooseDir)ret = TRUE;
		}
		else
		{
			if (chooseDir == 0 || chooseDir == 2)ret = TRUE;
		}
		if (ret)PathCombine(choosedPath, tempPath, fd[listpagecur + cur].cFileName);
		else goto tagCursorMove;
	}
	else if (keypressed == keyCancel)ret = FALSE;
	else switch (keypressed)
	{
	//Navigate fp,initpath,outpath
	case KEY_INPUT_UP:
		if (cur == 0)
		{
			if (listpagecur == 0)
			{
				cur = min(list_show_items - 1, ci - 1);
				listpagecur = max(0, ci - list_show_items);
			}
			else
			{
				listpagecur--;
			}
		}
		else
		{
			cur--;
		}
		goto tagCursorMove;
	case KEY_INPUT_DOWN:
		if (cur == list_show_items - 1)
		{
			if (listpagecur == ci - list_show_items)
			{
				cur = 0;
				listpagecur = 0;
			}
			else
			{
				listpagecur++;
			}
		}
		else if (cur == ci - 1)
		{
			cur = 0;
		}
		else
		{
			cur++;
		}
		goto tagCursorMove;
	case KEY_INPUT_LEFT:
		ParseFileNameFromPath(tempPath, findingInitPath);
		PathCombine(tempPath, tempPath, TEXT(".."));
		goto tagUpdateDir;
	case KEY_INPUT_RIGHT:
		if (fd[listpagecur + cur].dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			PathCombine(tempPath, tempPath, fd[listpagecur + cur].cFileName);
			goto tagUpdateDir;
		}
		else
		{
			goto tagCursorMove;
		}
	case KEY_INPUT_PGUP:
		for (int i = 0; i < list_show_items; i++)
			keyqueue.push_back(KEY_INPUT_UP);
		goto tagCursorMove;
	case KEY_INPUT_PGDN:
		for (int i = 0; i < list_show_items; i++)
			keyqueue.push_back(KEY_INPUT_DOWN);
		goto tagCursorMove;
	case KEY_INPUT_TAB:
		GetNextLogicalDriveString(tempPath, tempPath);
		goto tagUpdateDir;
	case KEY_INPUT_HOME:
		for (int i = listpagecur + cur; i > 0; i--)
			keyqueue.push_back(KEY_INPUT_UP);
		goto tagCursorMove;
	case KEY_INPUT_END:
		for (int i = ci - 1 - listpagecur - cur; i > 0; i--)
			keyqueue.push_back(KEY_INPUT_DOWN);
		goto tagCursorMove;
	default:goto tagCursorMove;
	}
	delete fd;
	DeleteFontToHandle(hDxFont);
	while (CheckHitKey(keypressed));
	return ret;
}
//cx,cy有虚值DXGUI_POSITION_CENTER表示屏幕中心
int DxGetInputString(const TCHAR *msg, TCHAR *outString, int limit, BOOL multiline, BOOL onlyNum, int strcolor, int bgcolor, int bordercolor,
	float borderwidth, const TCHAR *fontname, int fontsize, int fontthick, int cx, int cy, int paddingWidth, int paddingHeight)
{
	TCHAR msg_def[50];
	int hDxFont = DxShellCreateFontToHandle(fontname, fontsize, fontthick);
	if (hDxFont == -1)return FALSE;
	int ww, wh;
	GetDrawScreenSize(&ww, &wh);
	if (cx == DXGUI_POSITION_CENTER)cx = ww / 2;
	if (cy == DXGUI_POSITION_CENTER)cy = wh / 2;
	int len = 0;
	
	const int show_lines = 5;
	if (msg == NULL)
	{
		strcpyDx(msg_def, multiline ? LoadLocalString(IDS_DXGUI_GETINPUT_MULTILINE_MSG_DEFAULT) : LoadLocalString(IDS_DXGUI_GETINPUT_MSG_DEFAULT));
		msg = msg_def;
	}
	int strw, strh, lc, inputLineH, statusLineH;
	GetDrawStringSizeToHandle(&strw, &inputLineH, &lc, TEXT("高"), 1, hDxFont);//获取一行字的高度，也可以用singlelineh = strh / lc获取
	statusLineH = inputLineH;
	GetDrawStringSizeToHandle(&strw, &strh, &lc, msg, (int)strlenDx(msg), hDxFont);
	if (paddingWidth == -1)
		paddingWidth = (ww - strw) / 2;
	bool expandingPaddingHeight = false;
	if (paddingHeight == -1)
		expandingPaddingHeight = true;
	strw = max(strw, HdpiNum(DXGUI_DRAWSTRING_MIN_WIDTH));
	int inputHandle = MakeKeyInput(limit, TRUE, FALSE, onlyNum, FALSE, multiline);
	SetActiveKeyInput(inputHandle);
	SetKeyInputString(outString, inputHandle);
	SetKeyInputStringFont(hDxFont);
	cx = cx - strw / 2 - paddingWidth;
	RECT inputRect = { cx + paddingWidth,0,cx + paddingWidth + strw,0 };
	int keyInputState = 0, inputStrW, inputLineCount;
	TCHAR strChCount[24];
	int boxY;
	while (keyInputState == 0)
	{
		GetKeyInputString(outString, inputHandle);
		len = (int)strlenDx(outString);
		GetDrawStringSizeToHandle(&inputStrW, &inputLineH, &inputLineCount, outString, len, hDxFont);
		if (inputLineH == 0)
			inputLineH = statusLineH;
		else if (inputLineCount > show_lines)
			inputLineH = statusLineH * show_lines;
		if (multiline)
			inputLineH += statusLineH / 2;
		if (expandingPaddingHeight)
			paddingHeight = (wh - strh - inputLineH - statusLineH) / 2;
		boxY = cy - (strh + inputLineH + statusLineH) / 2 - paddingHeight;
		inputRect.top = boxY + paddingHeight + strh;
		inputRect.bottom = boxY + paddingHeight + strh + inputLineH;
		SetKeyInputDrawArea(inputRect.left, inputRect.top, inputRect.right, inputRect.bottom, inputHandle);
		DrawBox(cx, boxY, cx + strw + 2 * paddingWidth, boxY + strh + inputLineH + statusLineH + 2 * paddingHeight, bgcolor, TRUE);
		if (borderwidth > 0.0f)
			DrawBoxAA((float)cx, (float)boxY, cx + strw + 2.0f * paddingWidth, boxY + strh + inputLineH + statusLineH + 2.0f * paddingHeight,
				bordercolor, FALSE, borderwidth);
		DrawStringToHandle(cx + paddingWidth, boxY + paddingHeight, msg, strcolor, hDxFont);
		DrawBox(inputRect.left, inputRect.top, inputRect.right, inputRect.bottom, strcolor, FALSE);
		DrawKeyInputModeString(inputRect.left, inputRect.bottom);
		sprintfDx(strChCount, TEXT("%d/%d"), len, limit);
		GetDrawStringSizeToHandle(&inputStrW, &inputLineH, &inputLineCount, strChCount, (int)strlenDx(strChCount), hDxFont);
		DrawStringToHandle(inputRect.right - inputStrW, inputRect.bottom, strChCount, strcolor, hDxFont);
		DrawKeyInputString(inputRect.left, inputRect.top, inputHandle);
		ScreenFlip();
		ProcessMessage();//输入系统需要处理消息事件
		keyInputState = CheckKeyInput(inputHandle);
		if (multiline&&CheckHitKey(KEY_INPUT_RETURN))
			if (CheckHitKey(KEY_INPUT_LCONTROL) || CheckHitKey(KEY_INPUT_RCONTROL))
				keyInputState = 1;
	}
	DeleteKeyInput(inputHandle);
	DeleteFontToHandle(hDxFont);
	if (keyInputState == 2)
		len = -1;
	while (CheckHitKey(KEY_INPUT_ESCAPE) || CheckHitKey(KEY_INPUT_RETURN));
	return len;
}
