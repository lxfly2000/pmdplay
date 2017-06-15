#include <DxLib.h>
#include <locale>
#include "PMDPlayer.h"
#include "PMDScreen.h"
#include "DxKeyTrigger.h"
#include "ChooseFileDialog.h"
#include "DxShell.h"
#include "resource.h"
#include "Update.h"

#define IDM_APP_HELP	0x101
#define IDM_CONVERT		0x102
#define IDM_APP_UPDATE	0x103

#define APP_NAME	"Ｐrofessional Ｍusic Ｄriver (Ｐ.Ｍ.Ｄ.) Player"
#define HELP_PARAM	"-e <PMD文件名> [WAV文件名] [循环次数] [淡出时间(ms)]"
#define HELP_INFO	APP_NAME "\nBy lxfly2000\n\n* 播放时按ESC退出。\n* 如果要使用节奏声音，请将下列文件"\
					"\n  2608_bd.wav  2608_sd.wav  2608_top.wav\n  2608_hh.wav  2608_tom.wav 2608_rim.wav\n"\
					"  放至此程序目录下。（可由 -yr 命令获得）\n"\
					"* Wave 转换命令：\n  " HELP_PARAM "\n\n"\
					"本程序参考了以下代码：\n"\
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
#define NUM_SHOW_CHANNELS 9
#define DOUBLECLICK_BETWEEN_MS 500

const TCHAR projectURL[] = TEXT("https://github.com/lxfly2000/pmdplay");
const TCHAR updateFileURL[] = TEXT("https://raw.githubusercontent.com/lxfly2000/pmdplay/master/resource.h");
const TCHAR pcszPMDType[] = TEXT("PMD 文件\0*.m;*.m2;*.m26;*.m86;*.mz;*.mp;*.ms\0所有文件\0*\0\0");
#define USTR UpdateString(szStr, ARRAYSIZE(szStr), player.GetPlayerStatus() >= PMDPlayer::playing, filepath)

char strANSIbuf[MAX_PATH] = "";
char* A(const TCHAR* str)
{
	wcstombs(strANSIbuf, str, ARRAYSIZE(strANSIbuf));
	return strANSIbuf;
}

class DPIInfo
{
public:
	DPIInfo()
	{
		HDC h = GetDC(0);
		sx = GetDeviceCaps(h, LOGPIXELSX);
		sy = GetDeviceCaps(h, LOGPIXELSY);
	}
	template<typename Tnum>Tnum X(Tnum n)const { return n*sx / odpi; }
	template<typename Tnum>Tnum Y(Tnum n)const { return n*sy / odpi; }
private:
	const int odpi = 96;
	int sx, sy;
};

class ClickTrigger
{
public:
	ClickTrigger(int button):key(button),log(false){}
	bool KeyReleased()
	{
		clicking = GetMouseInput()&key;
		cv = log && !clicking;
		log = clicking;
		return cv;
	}
private:
	bool clicking, log, cv;
	int key;
};

class PMDPlay
{
public:
	PMDPlay();
	int Init(TCHAR* param);
	void Run();
	void Convert();
	void ConvertDialog();
	void CheckUpdate(bool showError);
	int End();
private:
	static LRESULT CALLBACK ExtraProcess(HWND, UINT, WPARAM, LPARAM);
	static PMDPlay* _pObj;
	static WNDPROC dxProcess;
	void OnDraw();
	void OnLoop();
	void OnDrop(HDROP hdrop);
	void OnAbout();
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
	ClickTrigger leftclick;
	int lastclicktime = 0, thisclicktime = 0;
	int retcode = 0;
};

PMDPlay::PMDPlay() :player(CHANNELS, SAMPLE_RATE, BYTES_PER_VAR, 20), leftclick(MOUSE_INPUT_LEFT)
{
	for (int i = 0; i < ARRAYSIZE(channelOn); i++)channelOn[i] = true;
}

PMDPlay* PMDPlay::_pObj = nullptr;
WNDPROC PMDPlay::dxProcess = nullptr;

int PMDPlay::Init(TCHAR* param)
{
	_pObj = this;
	setlocale(LC_ALL, "");
	//加载节奏声音
	if (!player.LoadRhythmFromDirectory("."))
	{
		const int files_id[] = {
			IDR_WAVE_2608_BD,IDR_WAVE_2608_SD,IDR_WAVE_2608_TOP,
			IDR_WAVE_2608_HH,IDR_WAVE_2608_TOM,IDR_WAVE_2608_RIM
		};
		char *file_mem[6] = { NULL };
		for (int i = 0; i < ARRAYSIZE(files_id); i++)
			file_mem[i] = (char*)LockResource(LoadResource(NULL,
				FindResource(NULL, MAKEINTRESOURCE(files_id[i]), TEXT("Wave"))));
		player.LoadRhythmFromMemory(file_mem[0], file_mem[1], file_mem[2], file_mem[3], file_mem[4], file_mem[5]);
	}

	//程序设定
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
		else if (stricmpDx(__wargv[1], TEXT("-e")) == 0)
			return 1;
	}
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(windowed = TRUE);
	SetWindowSizeChangeEnableFlag(TRUE);
	SetAlwaysRunFlag(TRUE);
	DPIInfo hdpi;
	SetGraphMode(hdpi.X(w), hdpi.Y(h), 32);
	ChangeFont(TEXT("SimSun"));
	SetFontSize(14);
	SetFontThickness(3);
	GetDrawScreenSize(&screenWidth, &screenHeight);
	if (DxLib_Init())return retcode = -1;
	SetDrawScreen(DX_SCREEN_BACK);
	hWindowDx = GetMainWindowHandle();
	dxProcess = (WNDPROC)GetWindowLongPtr(hWindowDx, GWLP_WNDPROC);
	SetWindowLongPtr(hWindowDx, GWL_EXSTYLE, WS_EX_ACCEPTFILES | GetWindowLongPtr(hWindowDx, GWL_EXSTYLE));
	SetWindowLongPtr(hWindowDx, GWLP_WNDPROC, (LONG_PTR)ExtraProcess);
	HMENU hSysMenu = GetSystemMenu(hWindowDx, FALSE);
	AppendMenu(hSysMenu, MF_STRING, IDM_CONVERT, TEXT("转换到 Wave(&C)……"));
	AppendMenu(hSysMenu, MF_STRING, IDM_APP_UPDATE, TEXT("检查更新(&U)……"));
	AppendMenu(hSysMenu, MF_STRING, IDM_APP_HELP, TEXT("关于本程序(&A)……\tF1"));
	SetWindowText(TEXT(APP_NAME));

	//界面显示设定
	posYLowerText = screenHeight - (GetFontSize() + 4) * 2;
	pmdscreen.SetKeyNotesSrc(&player, NUM_SHOW_CHANNELS);
	keydisp_x = 4;
	keydisp_y = 18;
	keydisp_w = screenWidth - 8;
	keydisp_h = posYLowerText - 18;
	keydisp_onechannel_h = keydisp_h / NUM_SHOW_CHANNELS;
	pmdscreen.SetRectangle(keydisp_x, keydisp_y, keydisp_w, keydisp_h);
	USTR;

	//参数播放
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

	CheckUpdate(false);
	return 0;
}

void PMDPlay::CheckUpdate(bool showError)
{
	int a;
	SetWindowText(TEXT("检查更新中……"));
	int r = CheckForUpdate(updateFileURL, &a);
	SetWindowText(TEXT(APP_NAME));
	if(showError)switch (r)
	{
	case -2:MessageBox(hWindowDx, TEXT("无法检查更新。\n\n可能的原因：\n"
		"* 你没有连接到网络；\n* 网络有问题；\n* 目标服务器已更改。"), NULL, MB_ICONERROR); break;
	case -1:MessageBox(hWindowDx, TEXT("无法检查更新。\n\n可能的原因：\n"
		"* URL 路径有误；\n* 更新配置文件不正确。"), NULL, MB_ICONERROR); break;
	case 0:MessageBox(hWindowDx, TEXT("该软件是最新的。"), TEXT(APP_NAME), MB_ICONINFORMATION); break;
	case 1:break;
	default:MessageBox(hWindowDx, TEXT("检查更新时出错。"), NULL, MB_ICONERROR); break;
	}
	if (r != 1)return;
	TCHAR msg[100] = TEXT("");
	wsprintf(msg, TEXT("检测到新版本：%d.%d.%d.%d\n当前版本：%s\n\n是否下载新版本？"),
		(a >> 24) & 0xFF, (a >> 16) & 0xFF, (a >> 8) & 0xFF, a & 0xFF, TEXT(APP_VERSION_STRING));
	if (MessageBox(hWindowDx, msg, TEXT(APP_NAME), MB_ICONQUESTION | MB_YESNO) == IDYES)
		ShellExecute(hWindowDx, TEXT("open"), projectURL, NULL, NULL, SW_SHOWNORMAL);
}

int PMDPlay::End()
{
	retcode |= DxLib_End();
	return retcode;
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

void PMDPlay::Convert()
{
	char srcfile[MAX_PATH] = "", outfile[MAX_PATH] = "";
	int loopcount = 1;
	int fadetime = 5000;
	switch (__argc)
	{
	case 6:fadetime = atoiDx(__wargv[5]);
	case 5:loopcount = atoiDx(__wargv[4]);
	case 4:strcpy(outfile, A(__wargv[3]));
	case 3:strcpy(srcfile, A(__wargv[2])); break;
	default:
		AppLogAdd(TEXT("参数错误。\n"));
		AppLogAdd(TEXT("参数格式为：" HELP_PARAM "\n"));
		return;
	}
	if (strcmp(outfile, "") == 0)
		sprintf(outfile, "%s.wav", srcfile);
	char *asrcfile = srcfile, *aoutfile = outfile;
	if (asrcfile[0] == '\"')
	{
		asrcfile[strlen(asrcfile) - 1] = 0;
		asrcfile++;
	}
	if (aoutfile[0] == '\"')
	{
		aoutfile[strlen(aoutfile) - 1] = 0;
		aoutfile++;
	}
	if (!player.Convert(asrcfile, aoutfile, loopcount, fadetime))
	{
		retcode = -1;
		AppLogAdd(TEXT("无法转换文件：%s\n"), __wargv[2]);
	}
}

void PMDPlay::ConvertDialog()
{
	TCHAR srcfile[MAX_PATH] = TEXT(""), pmdppath[MAX_PATH] = TEXT(""), cmd[1024] = TEXT("");
	strcpyDx(cmd, TEXT("-e "));
	strcpyDx(srcfile, filepath);//先给一个默认值，使用当前选择的文件
	if (!ChooseFile(hWindowDx, srcfile, NULL, pcszPMDType, NULL))return;//选择PMD文件
	strcatDx(cmd, srcfile);
	strcatDx(cmd, TEXT(" "));
	sprintfDx(pmdppath, TEXT("%s.WAV"), srcfile);
	if (!ChooseSaveFile(hWindowDx, pmdppath, NULL, TEXT("波形音频\0*.wav\0\0"), NULL))return;//选择保存文件
	strcatDx(cmd, pmdppath);
	SHELLEXECUTEINFO se = { 0 };
	se.cbSize = sizeof se;
	se.hwnd = hWindowDx;
	se.lpVerb = TEXT("open");
	se.lpFile = _wpgmptr;
	se.lpParameters = cmd;
	se.fMask = SEE_MASK_NOCLOSEPROCESS;
	SetWindowText(TEXT("转换中……"));
	ShellExecuteEx(&se);
	WaitForSingleObject(se.hProcess, INFINITE);
	GetExitCodeProcess(se.hProcess, (DWORD*)&retcode);
	CloseHandle(se.hProcess);
	SetWindowText(TEXT(APP_NAME));
	if (retcode)
	{
		MessageBox(hWindowDx, TEXT("转换失败。"), srcfile, MB_ICONERROR);
		return;
	}
	MessageBox(hWindowDx, TEXT("转换成功。\n如果需要调整循环次数等设置，请使用命令行转换。"), srcfile, MB_ICONINFORMATION);
}

LRESULT CALLBACK PMDPlay::ExtraProcess(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DROPFILES:PMDPlay::_pObj->OnDrop((HDROP)wp); break;
	case WM_SYSCOMMAND:
		switch (LOWORD(wp))
		{
		case IDM_APP_HELP:SendMessage(hwnd, WM_HELP, 0, 0); break;
		case IDM_CONVERT:_pObj->ConvertDialog(); break;
		case IDM_APP_UPDATE:_pObj->CheckUpdate(true); break;
		}
		break;
	case WM_HELP:PMDPlay::_pObj->OnAbout(); break;
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

void PMDPlay::OnAbout()
{
	if (windowed)
	{
		MessageBox(hWindowDx, TEXT(HELP_INFO), TEXT(APP_NAME), MB_ICONINFORMATION);
	}
	else
	{
		DxMessageBox(TEXT(HELP_INFO)TEXT("\n\n[Enter]继续"));
		while (CheckHitKey(KEY_INPUT_ESCAPE));
	}
}

bool PMDPlay::OnLoadFile(TCHAR *path)
{
	bool ok = true;
	if (!LoadFromString(path))ok = false;
	if (!ok)strcatDx(path, TEXT("（无效文件）"));
	USTR;
	player.SetPlaybackSpeed(1.0f);
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
			DrawFormatString(keydisp_x, keydisp_y + keydisp_onechannel_h*i, 0x00FFFFFF, TEXT("%3d %3d %3d"),
			player.GetKeysState()[i], player.GetKeyVoice()[i], player.GetKeyVolume()[i]);
	ScreenFlip();
}

void PMDPlay::OnLoop()
{
	//Esc
	if (KeyReleased(KEY_INPUT_ESCAPE))running = false;
	//F11
	if (KeyReleased(KEY_INPUT_F11))ChangeWindowMode(windowed ^= TRUE);
	//Space
	if (KeyReleased(KEY_INPUT_SPACE))OnCommandPlay();
	//S
	if (KeyReleased(KEY_INPUT_S) || player.GetPlayerStatus() == PMDPlayer::fadedout) { player.Unload(); USTR; }
	//F
	if (KeyReleased(KEY_INPUT_F)) { player.FadeoutAndStop(FADEOUT_TIME_SEC * 1000); USTR; }
	//O
	if (KeyReleased(KEY_INPUT_O))
	{
		if (windowed ? ChooseFile(hWindowDx, filepath, NULL, pcszPMDType, NULL) : DxChooseFilePath(filepath, filepath))
		{
			OnLoadFile(filepath);
		}
		else while (CheckHitKey(KEY_INPUT_ESCAPE));
	}
	//D
	if (KeyReleased(KEY_INPUT_D)) { showVoiceAndVolume = !showVoiceAndVolume; USTR; }
	//Left
	if (KeyReleased(KEY_INPUT_LEFT)&&player.GetPlayerStatus()!=PMDPlayer::nofile)setpos(max(getpos() - 5000, 0));
	//Right
	if (KeyReleased(KEY_INPUT_RIGHT)&&player.GetPlayerStatus()!=PMDPlayer::nofile)setpos(getpos() + 5000);
	//Up
	if (KeyReleased(KEY_INPUT_UP)) { player.SetVolume(player.GetVolume() + 5.0f); USTR; }
	//Down
	if (KeyReleased(KEY_INPUT_DOWN)) { player.SetVolume(player.GetVolume() - 5.0f); USTR; }
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
			if (!windowed)strcatDx(unicode_str, TEXT("\n[Enter]继续 [Space]切换Shift-JIS编码"));
			if (windowed ? MessageBox(hWindowDx, unicode_str, TEXT("文件信息（按取消切换 Shift-JIS 编码）"),
				MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL : DxMessageBox(unicode_str, KEY_INPUT_SPACE, KEY_INPUT_RETURN))
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
	if (KeyReleased(KEY_INPUT_Z))player.SetPlaybackSpeed(player.GetPlaybackSpeed()*2.0f);
	if (KeyReleased(KEY_INPUT_X))player.SetPlaybackSpeed(1.0f);
	if (KeyReleased(KEY_INPUT_C))player.SetPlaybackSpeed(player.GetPlaybackSpeed()/2.0f);
	if (leftclick.KeyReleased())
	{
		thisclicktime = GetNowCount();
		if (thisclicktime - lastclicktime < DOUBLECLICK_BETWEEN_MS)
			SetWindowSize(screenWidth, screenHeight);
		lastclicktime = thisclicktime;
	}
}

void PMDPlay::UpdateString(TCHAR *str, int strsize, bool isplaying, const TCHAR *path)
{
	if (strlenDx(path) > 80)
		path = strrchrDx(path, TEXT('\\')) + 1;
	snprintfDx(str, strsize, TEXT("Space:播放/暂停 S:停止 O:打开 F:淡出 I:文件信息 D:通道信息[%s] P:音色[%s] V:力度[%s] ↑↓:音量[%d%%]\n%s：%s"),
		showVoiceAndVolume ? TEXT("开") : TEXT("关"), pmdscreen.showVoice ? TEXT("开") : TEXT("关"),
		pmdscreen.showVolume ? TEXT("开") : TEXT("关"), (int)player.GetVolume(),
		isplaying ? TEXT("正在播放") : TEXT("当前文件"), path[0] ? path : TEXT("未选择"));
}

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	SetThreadUILanguage(GetUserDefaultUILanguage());
	if (lstrcmpi(TEXT("-yr"), lpCmdLine) == 0)
	{
		MessageBox(NULL, TEXT("本程序已经可以自动加载节奏声音了，无须释放文件。"), lpCmdLine, MB_ICONINFORMATION);
		return 0;
	}

	PMDPlay p;
	switch (p.Init(lpCmdLine))
	{
	case 0:p.Run(); break;
	case 1:p.Convert(); break;
	}
	return p.End();
}
