#include <fstream>
#include <DxLib.h>
#include "PMDPlayer.h"
#include "PMDScreen.h"
#include "..\MyCodes\TextEncodeConvert.h"
#include "..\MyCodes\DxKeyTrigger.h"
#include "..\MyCodes\ChooseFileDialog.h"
#include "YM2608_RhythmFiles.h"
#pragma comment(lib,"XAudio2.lib")

#define APP_NAME	"Professional Music Driver (P.M.D.) Player"
#define HELP_INFO	APP_NAME "\nBy lxfly2000\n\n* 播放时按ESC退出。\n* 如果要使用节奏声音，请将下列文件"\
					"\n\t2608_bd.wav\n\t2608_sd.wav\n\t2608_top.wav\n\t2608_hh.wav\n\t2608_tom.wav\n\t2608_rim.wav\n"\
					"  放至此程序目录下。（可由 -yr 命令获得）\n\n"\
					"本程序参考了以下来源：\n"\
					" Ｐrofessional Ｍusic Ｄriver (Ｐ.Ｍ.Ｄ.) - M.Kajihara\n"\
					" OPNA FM Generator - cisc\n"\
					" PPZ8 PCM Driver - UKKY\n"\
					" PMDWin - C60\n"\
					" pmdmini - BouKiCHi"
#define SAMPLE_RATE	44100//采样率（每秒多少个采样）
#define BYTES_PER_VAR	2//一个采样点的一个通道所占字节数
#define CHANNELS	2//通道数
#define BYTE_RATE	(SAMPLE_RATE*BYTES_PER_VAR*CHANNELS)//每秒有多少字节通过
#define FADEOUT_TIME_SEC	5
#define STEPS_PER_BAR	4
#define NUM_SHOW_CHANNELS 10

const TCHAR pcszPMDType[] = TEXT("PMD 文件\0*.m;*.m2\0所有文件\0*\0\0");
#define USTR UpdateString(szStr, ARRAYSIZE(szStr), player.GetPlayerStatus() >= PMDPlayer::playing, filepath)

char strANSIbuf[MAX_PATH] = "";
char* A(const TCHAR* str)
{
	UnicodeToANSI(strANSIbuf, ARRAYSIZE(strANSIbuf), str);
	return strANSIbuf;
}

class PMDPlay
{
public:
	PMDPlay();
	int Init(TCHAR* param);
	void Run();
	int End();
private:
	static LRESULT CALLBACK ExtraProcess(HWND, UINT, WPARAM, LPARAM);
	static PMDPlay* _pObj;
	static WNDPROC dxProcess;
	void OnDraw();
	void OnLoop();
	void OnDrop(HDROP hdrop);
	void OnCommandPlay();
	bool OnLoadFile(TCHAR* path);//加载文件

	bool LoadFromString(TCHAR*);
	void UpdateString(TCHAR* str, int strsize, bool isplaying, const TCHAR *path);
	void UpdateTextLastTime();
	void DrawTime();

	PMDPlayer player;
	PMDScreen pmdscreen;
	HWND hWindowDx;
	int windowed;
	int screenWidth, screenHeight;
	int posYLowerText;
	int minute = 0, second = 0, millisecond = 0;
	int xiaojie = 0, step = 0, tick = 0, ticksPerStep = 4;
	bool running = true;
	TCHAR filepath[MAX_PATH] = TEXT("");
	TCHAR szStr[200] = TEXT("");
	TCHAR szTimeInfo[80] = TEXT("");
	TCHAR szLastTime[10] = TEXT("0:00.000");
	bool channelOn[NumOfAllPart];
	bool showVoiceAndVolume = false;
	bool showVoiceOnKey = false;
	bool showVolumeOnKey = false;
	int keydisp_x, keydisp_y, keydisp_w, keydisp_h, keydisp_onechannel_h;
};

PMDPlay::PMDPlay() :player(CHANNELS, SAMPLE_RATE, BYTES_PER_VAR, 20)
{
	for (int i = 0; i < ARRAYSIZE(channelOn); i++)channelOn[i] = true;
}

PMDPlay* PMDPlay::_pObj = nullptr;
WNDPROC PMDPlay::dxProcess = nullptr;

int PMDPlay::Init(TCHAR* param)
{
	_pObj = this;
	int w = 800, h = 600;
	if (__argc > 1)
	{
		if (stricmpDx(__wargv[1], TEXT("600p")) == 0)
		{
			w = 960;
			h = 600;
			param[0] = 0;
		}
		else if (stricmpDx(__wargv[1], TEXT("720p")) == 0)
		{
			w = 1280;
			h = 720;
			param[0] = 0;
		}
	}
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(windowed = TRUE);
	SetAlwaysRunFlag(TRUE);
	SetGraphMode(w, h, 32);
	ChangeFont(TEXT("SimSun"));
	SetFontSize(14);
	SetFontThickness(3);
	GetDrawScreenSize(&screenWidth, &screenHeight);
	if (DxLib_Init())return -1;
	SetDrawScreen(DX_SCREEN_BACK);

	posYLowerText = screenHeight - (GetFontSize() + 4) * 2;

	pmdscreen.SetKeyNotesSrc(&player, NUM_SHOW_CHANNELS);
	keydisp_x = 4;
	keydisp_y = 18;
	keydisp_w = w - 8;
	keydisp_h = posYLowerText - 18;
	keydisp_onechannel_h = keydisp_h / NUM_SHOW_CHANNELS;
	pmdscreen.SetRectangle(keydisp_x, keydisp_y, keydisp_w, keydisp_h);
	USTR;

	if (param[0])
	{
		if (param[0] == '\"')
		{
			strcpyDx(filepath, param + 1);
			filepath[strlenDx(filepath) - 1] = 0;
		}
		else strcpyDx(filepath, param);
		if (OnLoadFile(filepath))OnCommandPlay();
	}

	hWindowDx = GetMainWindowHandle();
	dxProcess = (WNDPROC)GetWindowLongPtr(hWindowDx, GWLP_WNDPROC);
	SetWindowLongPtr(hWindowDx, GWL_EXSTYLE, WS_EX_ACCEPTFILES | GetWindowLongPtr(hWindowDx, GWL_EXSTYLE));
	SetWindowLongPtr(hWindowDx, GWLP_WNDPROC, (LONG_PTR)ExtraProcess);
	return 0;
}

int PMDPlay::End()
{
	return DxLib_End();
}

void PMDPlay::Run()
{
	while (running)
	{
		if (ProcessMessage())break;
		OnLoop();
		OnDraw();
	}
}

LRESULT CALLBACK PMDPlay::ExtraProcess(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DROPFILES:PMDPlay::_pObj->OnDrop((HDROP)wp); break;
	}
	return CallWindowProc(dxProcess, hwnd, msg, wp, lp);
}

void PMDPlay::OnCommandPlay()
{
	switch (player.GetPlayerStatus())
	{
	case PMDPlayer::playing:player.Pause(); break;
	case PMDPlayer::nofile:OnLoadFile(filepath);
	case PMDPlayer::paused:player.Play(); break;
	}
	USTR;
}

void PMDPlay::OnDrop(HDROP hdrop)
{
	DragQueryFile(hdrop, 0, filepath, MAX_PATH);
	DragFinish(hdrop);
	player.Unload();
	if (OnLoadFile(filepath))OnCommandPlay();
}

bool PMDPlay::OnLoadFile(TCHAR *path)
{
	bool ok = true;
	if (!LoadFromString(path))ok = false;
	if (!ok)strcatDx(path, TEXT("（无效文件）"));
	USTR;
	return ok;
}

bool PMDPlay::LoadFromString(TCHAR* str)
{
	if (player.LoadFromFile(A(str)))return false;
	UpdateTextLastTime();
	return true;
}

void PMDPlay::UpdateTextLastTime()
{
	millisecond = player.GetLengthInMs();
	second = millisecond / 1000;
	millisecond %= 1000;
	minute = second / 60;
	second %= 60;
	sprintfDx(szLastTime, TEXT("%d:%02d.%03d"), minute, second, millisecond);
	ticksPerStep = player.GetXiaojieLength() / STEPS_PER_BAR;
}

void PMDPlay::DrawTime()
{
	millisecond = player.GetPositionInMs();
	second = millisecond / 1000;
	millisecond %= 1000;
	minute = second / 60;
	second %= 60;
	tick = player.GetPositionInCount();
	step = tick / ticksPerStep;
	tick %= ticksPerStep;
	xiaojie = step / STEPS_PER_BAR;
	step %= STEPS_PER_BAR;
	sprintfDx(szTimeInfo, TEXT("BPM:%3d 循环：%2d 时间：%d:%02d.%03d/%s Tick:%3d:%d:%02d"), player.GetTempo(),
		player.GetLoopedTimes(), minute, second, millisecond, szLastTime, xiaojie, step, tick);
	DrawString(0, 0, szTimeInfo, 0x00FFFFFF);
}

void PMDPlay::OnDraw()
{
	ClearDrawScreen();
	pmdscreen.Draw();
	DrawTime();
	DrawString(0, posYLowerText, szStr, 0x00FFFFFF);
	if (showVoiceAndVolume)
		for (int i = 0; i < NUM_SHOW_CHANNELS; i++)
			DrawFormatString(keydisp_x, keydisp_y + keydisp_onechannel_h*i, 0x00FFFFFF, TEXT("%3d %3d"),
			player.GetKeyVoice()[i], player.GetKeyVolume()[i]);
	ScreenFlip();
}

void PMDPlay::OnLoop()
{
	//Esc
	if (KeyReleased(KEY_INPUT_ESCAPE))running = false;
	//F11
	if (KeyReleased(KEY_INPUT_F11))ChangeWindowMode(windowed ^= TRUE);
	//F1
	if (KeyReleased(KEY_INPUT_F1))MessageBox(hWindowDx, TEXT(HELP_INFO), TEXT(APP_NAME), MB_ICONINFORMATION);
	//Space
	if (KeyReleased(KEY_INPUT_SPACE))OnCommandPlay();
	//S
	if (KeyReleased(KEY_INPUT_S) || player.GetPlayerStatus() == PMDPlayer::fadedout) { player.Unload(); USTR; }
	//F
	if (KeyReleased(KEY_INPUT_F)) { player.FadeoutAndStop(FADEOUT_TIME_SEC * 1000); USTR; }
	//O
	if (KeyReleased(KEY_INPUT_O))
	{
		if (ChooseFile(hWindowDx, filepath, NULL, pcszPMDType, NULL))
		{
			OnLoadFile(filepath);
		}
	}
	//D
	if (KeyReleased(KEY_INPUT_D)) { showVoiceAndVolume = !showVoiceAndVolume; USTR; }
	//P
	if (KeyPressed(KEY_INPUT_P)) { pmdscreen.showVoice = !pmdscreen.showVoice; USTR; }
	//V
	if (KeyPressed(KEY_INPUT_V)) { pmdscreen.showVolume = !pmdscreen.showVolume; USTR; }
	//I
	if (KeyReleased(KEY_INPUT_I))
	{
		char info[1024] = "", singleinfo[80] = "";
		for (int i = -2; i < 10; i++)
		{
			if (player.GetNotes(singleinfo, i))return;
			strcat(info, singleinfo);
			if (singleinfo[0])strcat(info, "\n");
		}
		TCHAR unicode_str[1024];
		int codepage = CP_ACP;
		while (1)
		{
			MultiByteToWideChar(codepage, 0, info, ARRAYSIZE(info), unicode_str, ARRAYSIZE(unicode_str));
			if (MessageBox(hWindowDx, unicode_str, TEXT("文件信息（按取消切换 Shift-JIS 编码）"), MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL)
				codepage ^= 932;
			else break;
		}
	}
	for (int i = 0; i < 10; i++)
		if (KeyReleased(KEY_INPUT_1 + i))
		{
			channelOn[i] = !channelOn[i];
			if (i == 9)
				getopenwork()->effflag = !channelOn[i];
			else
				channelOn[i] ? maskoff(i) : maskon(i);
		}
	if (KeyReleased(KEY_INPUT_R))
		setrhythmwithssgeffect(channelOn[10] = !channelOn[10]);
}

void PMDPlay::UpdateString(TCHAR *str, int strsize, bool isplaying, const TCHAR *path)
{
	if (strlenDx(path) > 80)
		path = strrchrDx(path, TEXT('\\')) + 1;
	snprintfDx(str, strsize, TEXT("Space:播放/暂停 S:停止 O:打开文件 Esc:退出 F:淡出 I:文件信息 D:通道信息[%s] P:音色[%s] V:音量[%s]\n%s：%s"),
		showVoiceAndVolume ? TEXT("开") : TEXT("关"), pmdscreen.showVoice ? TEXT("开") : TEXT("关"),
		pmdscreen.showVolume ? TEXT("开") : TEXT("关"),
		isplaying ? TEXT("正在播放") : TEXT("当前文件"), path[0] ? path : TEXT("未选择"));
}

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	SetThreadUILanguage(GetUserDefaultUILanguage());
	if (lstrcmpi(TEXT("-yr"), lpCmdLine) == 0)
	{
		std::ofstream f;
		for (int i = 0; i < ARRAYSIZE(ym2608_files) / 2; i++)
		{
			f.open(ym2608_files[i * 2], std::ios::binary | std::ios::out);
			if (f)f.write(ym2608_files[i * 2 + 1], ym2608_files_size[i]);
			f.close();
		}
		return 0;
	}

	PMDPlay p;
	if (p.Init(lpCmdLine) == 0)
		p.Run();
	return p.End();
}
