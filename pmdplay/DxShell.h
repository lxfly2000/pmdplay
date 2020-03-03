#pragma once
#include<DxLib.h>
#define DXGUI_COLOR_STRING_DEFAULT		0x00FFFFFF
#define DXGUI_COLOR_BACKGROUND_DEFAULT	0x00000000
#define DXGUI_COLOR_BORDER_DEFAULT		0x00FFFFFF
#define DXGUI_BORDER_WIDTH_DEFAULT		0.0f
#define DXGUI_FONTSIZE_DEFAULT			-1
#define DXGUI_FONTNAME_DEFAULT			"宋体"
#define DXGUI_FONTTHICK_DEFAULT			-1
#define DXGUI_POSITION_CENTER			0x80000000
#define DXGUI_PADDING_WIDTH_DEFAULT		20
#define DXGUI_CHOOSEFILE_MSG_DEFAULT	"选择文件\n[方向键]移动 [Tab]盘符 [Enter]确定 [Esc]取消"
#define DXGUI_GETINPUT_MSG_DEFAULT		"输入\n[Enter]确定 [Esc]取消"
#define DXGUI_GETINPUT_MULTILINE_MSG_DEFAULT	"输入\n[Ctrl+Enter]确定 [Esc]取消"
#define DXGUI_DRAWSTRING_MIN_WIDTH		320
//显示一段提示信息，可以多行，返回值为键盘上按下的键（任何键均返回）
//strcolor：文字颜色
//bgcolor：背景颜色
//cx，cy：消息框中心位置，-1为屏幕中心
//fontsize：字体大小
//paddingWidth，paddingHeight：文字与边缘的距离（像素），-1为扩展至屏幕边缘
//返回值：按下keyOk键返回TRUE，keyCancel返回FALSE
int DxMessageBox(const TCHAR *msg, int keyOk = KEY_INPUT_RETURN, int keyCancel = KEY_INPUT_ESCAPE,
	int strcolor = DXGUI_COLOR_STRING_DEFAULT, int bgcolor = DXGUI_COLOR_BACKGROUND_DEFAULT,
	int bordercolor = DXGUI_COLOR_BORDER_DEFAULT, float borderwidth = DXGUI_BORDER_WIDTH_DEFAULT,
	const TCHAR *fontname = TEXT(DXGUI_FONTNAME_DEFAULT), int fontsize = DXGUI_FONTSIZE_DEFAULT,
	int fontthick = DXGUI_FONTTHICK_DEFAULT, int cx = DXGUI_POSITION_CENTER, int cy = DXGUI_POSITION_CENTER,
	int paddingWidth = DXGUI_PADDING_WIDTH_DEFAULT, int paddingHeight = DXGUI_PADDING_WIDTH_DEFAULT);
//获取选择文件的路径，文件或目录均可，按下Enter时返回TRUE，按下Esc时返回FALSE
//方向键选择目录
//initPath：初始路径
//choosedPath：选择的路径
//chooseDir：0为选择文件，1为选择目录，2为文件或目录均可
//strcolor：文字颜色
//bgcolor：背景颜色
//cx，cy：消息框中心位置，-1为屏幕中心
//fontsize：字体大小
//paddingWidth，paddingHeight：文字与边缘的距离（像素），-1为扩展至屏幕边缘
//返回值：按下keyOk键返回TRUE，keyCancel返回FALSE
int DxChooseFilePath(const TCHAR *initPath, TCHAR *choosedPath, const TCHAR *msg = TEXT(DXGUI_CHOOSEFILE_MSG_DEFAULT),
	int chooseDir = 0, int keyOk = KEY_INPUT_RETURN, int keyCancel = KEY_INPUT_ESCAPE, int strcolor = DXGUI_COLOR_STRING_DEFAULT,
	int bgcolor = DXGUI_COLOR_BACKGROUND_DEFAULT, int bordercolor = DXGUI_COLOR_BORDER_DEFAULT, float borderwidth = DXGUI_BORDER_WIDTH_DEFAULT,
	const TCHAR *fontname = TEXT(DXGUI_FONTNAME_DEFAULT),
	int fontsize = DXGUI_FONTSIZE_DEFAULT, int fontthick = DXGUI_FONTTHICK_DEFAULT, int cx = DXGUI_POSITION_CENTER,
	int cy = DXGUI_POSITION_CENTER, int paddingWidth = DXGUI_PADDING_WIDTH_DEFAULT, int paddingHeight = DXGUI_PADDING_WIDTH_DEFAULT);
//获取用户输入的一串文字，limit为长度上限，Enter返回输入的字符数量，Esc返回-1
//msg：提示文字，如果指定为NULL会使用默认值DXGUI_GETINPUT_MSG_DEFAULT
//outString：输出字符串，可以指定默认值
//strcolor：文字颜色
//bgcolor：背景颜色
//cx，cy：消息框中心位置，-1为屏幕中心
//fontsize：字体大小
//paddingWidth，paddingHeight：文字与边缘的距离（像素），-1为扩展至屏幕边缘
//返回值：按下ENTER键返回输入的字符串长度，ESC返回-1，但无论何种情况下outString都会被更新
int DxGetInputString(const TCHAR *msg, TCHAR *outString, int limit, BOOL multiline = FALSE, BOOL onlyNum = FALSE, int strcolor = DXGUI_COLOR_STRING_DEFAULT,
	int bgcolor = DXGUI_COLOR_BACKGROUND_DEFAULT, int bordercolor = DXGUI_COLOR_BORDER_DEFAULT, float borderwidth = DXGUI_BORDER_WIDTH_DEFAULT,
	const TCHAR *fontname = TEXT(DXGUI_FONTNAME_DEFAULT),
	int fontsize = DXGUI_FONTSIZE_DEFAULT, int fontthick = DXGUI_FONTTHICK_DEFAULT, int cx = DXGUI_POSITION_CENTER,
	int cy = DXGUI_POSITION_CENTER, int paddingWidth = DXGUI_PADDING_WIDTH_DEFAULT, int paddingHeight = DXGUI_PADDING_WIDTH_DEFAULT);
//缩短路径以使字符串的绘制宽度小于maxWidth
//返回值为shortend
TCHAR *ShortenPath(const TCHAR *src, BOOL isDir, TCHAR *shortened, int fontHandle, int maxWidth);
//设置DxShell是否使用高分辨率，默认为使用
void DxShellSetUseHighDpi(bool);
