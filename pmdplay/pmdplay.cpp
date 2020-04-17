#include <DxLib.h>
#include <locale>
#include "PMDPlayer_XAudio2.h"
#include "PMDPlayer_DSound.h"
#include "PMDScreen.h"
#include "DxKeyTrigger.h"
#include "ChooseFileDialog.h"
#include "DxShell.h"
#include "resource.h"
#include "Update.h"
#include "InputBox.h"
#include "viewmem.h"
#include "ResLoader.h"
#include "resource1.h"

#define IDM_APP_HELP	0x101
#define IDM_CONVERT		0x102
#define IDM_APP_UPDATE	0x103
#define IDM_BEATS_PER_BAR	0x104

#define APP_NAME	"Ｐrofessional Ｍusic Ｄriver (Ｐ.Ｍ.Ｄ.) Player"
LPCTSTR GetHelpParam()
{
	return LoadLocalString(IDS_HELP_PARAM);
}
TCHAR _helpInfo[700];
LPCTSTR GetHelpInfo()
{
	strcpyDx(_helpInfo, LoadLocalString(IDS_HELP_INFO_FMT));
	sprintfDx(_helpInfo, _helpInfo, TEXT(APP_NAME), GetHelpParam());
	return _helpInfo;
}
#define SAMPLE_RATE	44100//采样率（每秒多少个采样）
#define BYTES_PER_VAR	2//一个采样点的一个通道所占字节数
#define CHANNELS	2//通道数
#define BYTE_RATE	(SAMPLE_RATE*BYTES_PER_VAR*CHANNELS)//每秒有多少字节通过
#define FADEOUT_TIME_SEC	5
#define STEPS_PER_BAR	4//默认节拍数
#define NUM_SHOW_CHANNELS 9

LPCTSTR GetTypeString(LPTSTR out, UINT id)
{
	strcpyDx(out, LoadLocalString(id));
	size_t fixedLen = strlenDx(out);
	for (size_t i = 0; i < fixedLen; i++)
		if (out[i] == '|')
			out[i] = 0;
	return out;
}
TCHAR _pcszPMDType[80];
LPCTSTR GetPMDType()
{
	return GetTypeString(_pcszPMDType, IDS_PMD_TYPE);
}
#define USTR UpdateString(szStr, ARRAYSIZE(szStr), pplayer->GetPlayerStatus() >= PMDPlayer::playing, filepath)

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
	void ChangeBeatsPerBar();
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

	PMDPlayer *pplayer;
	PMDScreen pmdscreen;
	HWND hWindowDx;
	int windowed;
	int screenWidth, screenHeight;
	int posYLowerText;
	int minute = 0, second = 0, millisecond = 0;
	int xiaojie = 0, step = 0, tick = 0, ticksPerStep = 4, stepsPerBar = STEPS_PER_BAR;
	bool running = true;
	TCHAR filepath[MAX_PATH] = TEXT("");
	TCHAR szStr[380] = TEXT("");
	TCHAR szTimeInfo[80] = TEXT("");
	TCHAR szTimeInfo_fmt[60];
	TCHAR szLastTime[10] = TEXT("0:00.000");
	bool channelOn[PMDPLAYER_CHANNELS_NUM];
	bool showVoiceAndVolume = false;
	bool showVoiceOnKey = false;
	bool showVolumeOnKey = false;
	int keydisp_x, keydisp_y, keydisp_w, keydisp_h, keydisp_onechannel_h;
	ClickTrigger leftclick;
	int lastclicktime = 0, thisclicktime = 0;
	int retcode = 0;
	bool fileload_ok = true;
	int originalWinWidth, originalWinHeight, displayWinWidth, displayWinHeight;
	int mouseDoubleClickTime;
};

PMDPlay::PMDPlay() :leftclick(MOUSE_INPUT_LEFT)
{
	for (int i = 0; i < ARRAYSIZE(channelOn); i++)channelOn[i] = true;
}

PMDPlay* PMDPlay::_pObj = nullptr;
WNDPROC PMDPlay::dxProcess = nullptr;

int PMDPlay::Init(TCHAR* param)
{
	_pObj = this;
	setlocale(LC_ALL, "");
	pplayer = nullptr;
	bool isXAudio2 = false;
	if (!GetPrivateProfileInt(TEXT(SECTION_NAME), TEXT(KEYNAME_NO_XAUDIO2), 0, TEXT(PROFILE_NAME)))
	{
		pplayer = new PMDPlayer_XAudio2;
		if (pplayer->Init(CHANNELS, SAMPLE_RATE, BYTES_PER_VAR, 0))
		{
			pplayer->Release();
			delete pplayer;
			pplayer = nullptr;
		}
		else
			isXAudio2 = true;
	}
	if (pplayer == nullptr)
	{
		pplayer = new PMDPlayer_DSound;
		pplayer->Init(CHANNELS, SAMPLE_RATE, BYTES_PER_VAR, 0);
	}
	TCHAR rhydir[] = _T(".");
	//加载节奏声音
	if (!pplayer->LoadRhythmFromDirectory(rhydir))
	{
		const int files_id[] = {
			IDR_WAVE_2608_BD,IDR_WAVE_2608_SD,IDR_WAVE_2608_TOP,
			IDR_WAVE_2608_HH,IDR_WAVE_2608_TOM,IDR_WAVE_2608_RIM
		};
		char *file_mem[6] = { NULL };
		for (int i = 0; i < ARRAYSIZE(files_id); i++)
			file_mem[i] = (char*)LockResource(LoadResource(NULL,
				FindResource(NULL, MAKEINTRESOURCE(files_id[i]), TEXT("Wave"))));
		pplayer->LoadRhythmFromMemory(file_mem[0], file_mem[1], file_mem[2], file_mem[3], file_mem[4], file_mem[5]);
	}

	//程序设定
	mouseDoubleClickTime = (int)GetDoubleClickTime();
	originalWinWidth = 800, originalWinHeight = 600;
	bool paramPlay = true;
	if (strstrDx(param, TEXT("600p")))
	{
		originalWinWidth = 960;
		originalWinHeight = 600;
		paramPlay = false;
	}
	else if (strstrDx(param, TEXT("720p")))
	{
		originalWinWidth = 1280;
		originalWinHeight = 720;
		paramPlay = false;
	}
	else if (strstrDx(param, TEXT("-e")))
		return 1;
	bool useHighDpi = strstrDx(param, TEXT("hdpi"));
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(windowed = TRUE);
	SetWindowSizeChangeEnableFlag(TRUE);
	SetAlwaysRunFlag(TRUE);
	DPIInfo hdpi;
	if (useHighDpi)
	{
		displayWinWidth = hdpi.X(originalWinWidth);
		displayWinHeight = hdpi.Y(originalWinHeight);
		paramPlay = false;
	}
	else
	{
		displayWinWidth = originalWinWidth;
		displayWinHeight = originalWinHeight;
	}
	DxShellSetUseHighDpi(useHighDpi);
	SetGraphMode(displayWinWidth, displayWinHeight, 32);
	ChangeFont(TEXT("SimSun"));
	if (useHighDpi)
	{
		SetFontSize(hdpi.X(14));
		if (hdpi.X(14) > 14)
			ChangeFontType(DX_FONTTYPE_ANTIALIASING);
	}
	else
	{
		SetFontSize(14);
		SetWindowSize(hdpi.X(displayWinWidth), hdpi.Y(displayWinHeight));
	}
	SetFontThickness(3);
	GetDrawScreenSize(&screenWidth, &screenHeight);
	if (DxLib_Init())return retcode = -1;
	SetDrawScreen(DX_SCREEN_BACK);
	hWindowDx = GetMainWindowHandle();
	dxProcess = (WNDPROC)GetWindowLongPtr(hWindowDx, GWLP_WNDPROC);
	SetWindowLongPtr(hWindowDx, GWL_EXSTYLE, WS_EX_ACCEPTFILES | GetWindowLongPtr(hWindowDx, GWL_EXSTYLE));
	SetWindowLongPtr(hWindowDx, GWLP_WNDPROC, (LONG_PTR)ExtraProcess);
	HMENU hSysMenu = GetSystemMenu(hWindowDx, FALSE);
	AppendMenu(hSysMenu, MF_STRING, IDM_BEATS_PER_BAR, LoadLocalString(IDS_MENU_BEATS_PER_BAR));
	AppendMenu(hSysMenu, MF_STRING, IDM_CONVERT, LoadLocalString(IDS_MENU_CONVERT));
	AppendMenu(hSysMenu, MF_STRING, IDM_APP_UPDATE, LoadLocalString(IDS_MENU_CHECK_UPDATE));
	AppendMenu(hSysMenu, MF_STRING, IDM_APP_HELP, LoadLocalString(IDS_MENU_ABOUT));
	TCHAR ti[400];
	strcpyDx(ti, TEXT(APP_NAME));
	if (isXAudio2)
		strcatDx(ti, TEXT(" (XAudio2)"));
	else
		strcatDx(ti, TEXT(" (DSound)"));
	SetWindowText(ti);
	ViewMemInit();

	//界面显示设定
	posYLowerText = screenHeight - (GetFontSize() + 4) * 2;
	pmdscreen.SetKeyNotesSrc(pplayer, NUM_SHOW_CHANNELS);
	keydisp_x = 4;
	keydisp_y = GetFontSize() + 4;
	keydisp_w = screenWidth - 8;
	keydisp_h = posYLowerText - keydisp_y;
	keydisp_onechannel_h = keydisp_h / NUM_SHOW_CHANNELS;
	pmdscreen.SetRectangle(keydisp_x, keydisp_y, keydisp_w, keydisp_h);
	strcpyDx(szTimeInfo_fmt, LoadLocalString(IDS_DRAWTIME_FMT));
	USTR;

	//参数播放
	if (paramPlay && param[0])
	{
		if (param[0] == '\"')
		{
			strcpyDx(filepath, param + 1);
			filepath[strlenDx(filepath) - 1] = 0;
		}
		else strcpyDx(filepath, param);
		if (OnLoadFile(filepath))OnCommandPlay();
	}

	return 0;
}

void PMDPlay::CheckUpdate(bool showError)
{
	std::thread([=]()
	{
		int a;
		TCHAR ti[400];
		GetWindowText(GetMainWindowHandle(), ti, ARRAYSIZE(ti) - 1);
		SetWindowText(LoadLocalString(IDS_CHECKING_UPDATE));
		int r = CheckForUpdate(TEXT(UPDATE_FILE_URL), &a);
		SetWindowText(ti);
		if (showError)switch (r)
		{
		case -2:MessageBox(hWindowDx, LoadLocalString(IDS_CHECK_UPDATE_ERROR_NETWORK), NULL, MB_ICONERROR); break;
		case -1:MessageBox(hWindowDx, LoadLocalString(IDS_CHECK_UPDATE_ERROR_CONFIGURATION), NULL, MB_ICONERROR); break;
		case 0:MessageBox(hWindowDx, LoadLocalString(IDS_CHECK_UPDATE_LATEST), TEXT(APP_NAME), MB_ICONINFORMATION); break;
		case 1:break;
		default:MessageBox(hWindowDx, LoadLocalString(IDS_CHECK_UPDATE_ERROR_MISC), NULL, MB_ICONERROR); break;
		}
		if (r != 1)return;
		TCHAR msg[100] = TEXT("");
		wsprintf(msg, LoadLocalString(IDS_CHECK_UPDATE_NEW_VERSION),
			(a >> 24) & 0xFF, (a >> 16) & 0xFF, (a >> 8) & 0xFF, a & 0xFF, TEXT(APP_VERSION_STRING));
		if (MessageBox(hWindowDx, msg, TEXT(APP_NAME), MB_ICONQUESTION | MB_YESNO) == IDYES)
			ShellExecute(hWindowDx, TEXT("open"), TEXT(PROJECT_URL), NULL, NULL, SW_SHOWNORMAL);
	}).detach();
}

void PMDPlay::ChangeBeatsPerBar()
{
	TCHAR input[3];
	int _spb;
	do {
		wsprintf(input, TEXT("%d"), stepsPerBar);
		if (windowed)
		{
			TCHAR strOK[16], strCancel[16];
			strcpyDx(strOK, LoadLocalString(IDS_OK));
			strcpyDx(strCancel, LoadLocalString(IDS_CANCEL));
			if (!InputBox(hWindowDx, input, ARRAYSIZE(input), LoadLocalString(IDS_MSG_BPB), NULL, input, NULL, strOK, strCancel))return;
		}
		else
		{
			if (DxGetInputString(LoadLocalString(IDS_DXMSG_BPB), input, ARRAYSIZE(input) - 1, FALSE, TRUE) == -1)return;
		}
		_spb = _ttoi(input);
	} while (!_spb);
	stepsPerBar = _spb;
}

int PMDPlay::End()
{
	retcode |= DxLib_End();
	pplayer->Release();
	delete pplayer;
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
	TCHAR srcfile[MAX_PATH] = _T(""), outfile[MAX_PATH] = _T("");
	int loopcount = 1;
	int fadetime = 5000;
	bool split = false;
	switch (__argc)
	{
	case 7:if (lstrcmpi(__wargv[6], TEXT("-st")) == 0)split = true;
	case 6:fadetime = atoiDx(__wargv[5]);
	case 5:loopcount = atoiDx(__wargv[4]);
	case 4:lstrcpy(outfile, __wargv[3]);
	case 3:lstrcpy(srcfile, __wargv[2]); break;
	default:
	{
		TCHAR logmsg[120];
		strcpyDx(logmsg, LoadLocalString(IDS_LOG_ERROR_PARAM));
		strcatDx(logmsg, TEXT("\n"));
		AppLogAdd(logmsg);
		strcpyDx(logmsg, LoadLocalString(IDS_LOG_COMMAND));
		strcatDx(logmsg, GetHelpParam());
		strcatDx(logmsg, TEXT("\n"));
		AppLogAdd(logmsg);
	}
		return;
	}
	if (lstrcmp(outfile, _T("")) == 0)
		wsprintf(outfile, _T("%s.wav"), srcfile);
	TCHAR*asrcfile = srcfile, *aoutfile = outfile;
	if (asrcfile[0] == '\"')
	{
		asrcfile[lstrlen(asrcfile) - 1] = 0;
		asrcfile++;
	}
	if (aoutfile[0] == '\"')
	{
		aoutfile[lstrlen(aoutfile) - 1] = 0;
		aoutfile++;
	}
	if (!pplayer->Convert(asrcfile, aoutfile, loopcount, fadetime, split))
	{
		retcode = -1;
		TCHAR logmsg[300];
		strcpyDx(logmsg, LoadLocalString(IDS_LOG_CANNOT_CONVERT));
		strcatDx(logmsg, __wargv[2]);
		strcatDx(logmsg, TEXT("\n"));
		AppLogAdd(logmsg);
	}
}

void PMDPlay::ConvertDialog()
{
	TCHAR srcfile[MAX_PATH] = TEXT(""), pmdppath[MAX_PATH] = TEXT(""), cmd[1024] = TEXT("");
	strcpyDx(cmd, TEXT("-e "));
	strcpyDx(srcfile, filepath);//先给一个默认值，使用当前选择的文件
	if (windowed)
	{
		if (!ChooseFile(hWindowDx, srcfile, NULL, GetPMDType(), NULL))//选择PMD文件
			return;
	}
	else
	{
		if (!DxChooseFilePath(srcfile, srcfile))
			return;
	}
	strcatDx(cmd, srcfile);
	strcatDx(cmd, TEXT(" "));
	sprintfDx(pmdppath, TEXT("%s.WAV"), srcfile);
	BOOL st;
	if (windowed)
	{
		TCHAR ts[20];
		GetTypeString(ts, IDS_WAVE_TYPE);
		if (!ChooseSaveFileWithCheckBox(hWindowDx, pmdppath, NULL, ts, NULL, &st, LoadLocalString(IDS_SPLIT_CHANNEL)))//选择保存文件
			return;
	}
	else
	{
		if (DxGetInputString(LoadLocalString(IDS_DXMSG_INPUT_SAVE_PATH), pmdppath, ARRAYSIZE(pmdppath) - 1) == -1)
			return;
		st = DxMessageBox(LoadLocalString(IDS_DXMSG_SPLIT_CHANNEL), KEY_INPUT_Y, KEY_INPUT_N);
	}
	strcatDx(cmd, pmdppath);
	if (st)strcatDx(cmd, TEXT(" 1 5000 -st"));
	SHELLEXECUTEINFO se = { 0 };
	se.cbSize = sizeof se;
	se.hwnd = hWindowDx;
	se.lpVerb = TEXT("open");
	se.lpFile = _wpgmptr;
	se.lpParameters = cmd;
	se.fMask = SEE_MASK_NOCLOSEPROCESS;
	TCHAR ti[400];
	GetWindowText(GetMainWindowHandle(), ti, ARRAYSIZE(ti) - 1);
	SetWindowText(LoadLocalString(IDS_CONVERTING));
	ShellExecuteEx(&se);
	WaitForSingleObject(se.hProcess, INFINITE);
	GetExitCodeProcess(se.hProcess, (DWORD*)&retcode);
	CloseHandle(se.hProcess);
	SetWindowText(ti);
	if (retcode)
	{
		if (windowed)
			MessageBox(hWindowDx, LoadLocalString(IDS_MSG_CONVERT_FAILED), srcfile, MB_ICONERROR);
		else
			DxMessageBox(LoadLocalString(IDS_DXMSG_CONVERT_FAILED));
		return;
	}
	if (windowed)
		MessageBox(hWindowDx, LoadLocalString(IDS_MSG_CONVERT_SUCCEEDED), srcfile, MB_ICONINFORMATION);
	else
		DxMessageBox(LoadLocalString(IDS_DXMSG_CONVERT_SUCCEEDED));
}

LRESULT CALLBACK PMDPlay::ExtraProcess(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DROPFILES:PMDPlay::_pObj->OnDrop((HDROP)wp); break;
	case WM_SYSCOMMAND:
		switch (LOWORD(wp))
		{
		case IDM_APP_HELP:_pObj->OnAbout(); break;
		case IDM_CONVERT:_pObj->ConvertDialog(); break;
		case IDM_APP_UPDATE:_pObj->CheckUpdate(true); break;
		case IDM_BEATS_PER_BAR:_pObj->ChangeBeatsPerBar(); break;
		}
		break;
	}
	return CallWindowProc(dxProcess, hwnd, msg, wp, lp);
}

void PMDPlay::OnCommandPlay()
{
	switch (pplayer->GetPlayerStatus())
	{
	case PMDPlayer::playing:pplayer->Pause(); break;
	case PMDPlayer::nofile:OnLoadFile(filepath);
	case PMDPlayer::paused:pplayer->Play(); break;
	}
	USTR;
}

void PMDPlay::OnDrop(HDROP hdrop)
{
	DragQueryFile(hdrop, 0, filepath, MAX_PATH);
	DragFinish(hdrop);
	pplayer->Unload();
	if (OnLoadFile(filepath))OnCommandPlay();
}

void PMDPlay::OnAbout()
{
	if (windowed)
	{
		MessageBox(hWindowDx, GetHelpInfo(), TEXT(APP_NAME), MB_ICONINFORMATION);
	}
	else
	{
		TCHAR unicode_str[1024];
		strcpyDx(unicode_str, GetHelpInfo());
		strcatDx(unicode_str, LoadLocalString(IDS_DXMSG_APPEND_OK));
		DxMessageBox(unicode_str);
	}
}

bool PMDPlay::OnLoadFile(TCHAR *path)
{
	fileload_ok = true;
	if (!LoadFromString(path))fileload_ok = false;
	USTR;
	pplayer->SetPlaybackSpeed(1.0f);
	return fileload_ok;
}

bool PMDPlay::LoadFromString(TCHAR* str)
{
	if (pplayer->LoadFromFile(str))return false;
	UpdateTextLastTime();
	return true;
}

void PMDPlay::UpdateTextLastTime()
{
	millisecond = pplayer->GetLengthInMs();
	second = millisecond / 1000;
	millisecond %= 1000;
	minute = second / 60;
	second %= 60;
	sprintfDx(szLastTime, TEXT("%d:%02d.%03d"), minute, second, millisecond);
	ticksPerStep = pplayer->GetXiaojieLength() / stepsPerBar;
}

void PMDPlay::DrawTime()
{
	millisecond = pplayer->GetPositionInMs();
	second = millisecond / 1000;
	millisecond %= 1000;
	minute = second / 60;
	second %= 60;
	tick = pplayer->GetPositionInCount();
	step = tick / ticksPerStep;
	tick %= ticksPerStep;
	xiaojie = step / stepsPerBar;
	step %= stepsPerBar;
	sprintfDx(szTimeInfo, szTimeInfo_fmt, pplayer->GetTempo(),
		pplayer->GetLoopedTimes(), minute, second, millisecond, szLastTime, xiaojie, step, tick);
	DrawString(0, 0, szTimeInfo, 0x00FFFFFF);
}

void PMDPlay::OnDraw()
{
	ClearDrawScreen();
	pmdscreen.Draw();
	DrawTime();
	DrawString(0, posYLowerText, szStr, 0x00FFFFFF);
	static TCHAR drawbuf[16];
	if (showVoiceAndVolume)
	{
		for (int i = 0; i < NUM_SHOW_CHANNELS; i++)
		{
			if (pplayer->GetKeysState()[i] == -1)
				strcpyDx(drawbuf, TEXT("  -"));
			else
				sprintfDx(drawbuf, TEXT("%3d"), pplayer->GetKeysState()[i]);
			DrawFormatString(keydisp_x, keydisp_y + keydisp_onechannel_h * i, 0x00FFFFFF, TEXT("%s %3d %3d"),
				drawbuf, pplayer->GetKeyVoice()[i], pplayer->GetKeyVolume()[i]);
		}
	}
	ViewMemDraw();
	ScreenFlip();
}

void PMDPlay::OnLoop()
{
	//Esc
	if (KeyReleased(KEY_INPUT_ESCAPE))running = false;
	//F1
	if (KeyReleased(KEY_INPUT_F1))OnAbout();
	//F11
	if (KeyReleased(KEY_INPUT_F11))ChangeWindowMode(windowed ^= TRUE);
	//Space
	if (KeyReleased(KEY_INPUT_SPACE))OnCommandPlay();
	//S
	if (KeyReleased(KEY_INPUT_S) || pplayer->GetPlayerStatus() == PMDPlayer::fadedout) { pplayer->Unload(); USTR; }
	//F
	if (KeyReleased(KEY_INPUT_F)) { pplayer->FadeoutAndStop(FADEOUT_TIME_SEC * 1000); USTR; }
	//O
	if (KeyReleased(KEY_INPUT_O))
	{
		if (windowed ? ChooseFile(hWindowDx, filepath, NULL, GetPMDType(), NULL) : DxChooseFilePath(filepath, filepath))
		{
			OnLoadFile(filepath);
		}
	}
	//D
	if (KeyReleased(KEY_INPUT_D)) { showVoiceAndVolume = !showVoiceAndVolume; USTR; }
	//Left
	if (KeyReleased(KEY_INPUT_LEFT)&& pplayer->GetPlayerStatus()!=PMDPlayer::nofile)pplayer->SetPositionInMs(max(pplayer->GetPositionInMs() - 5000, 0));
	//Right
	if (KeyReleased(KEY_INPUT_RIGHT)&& pplayer->GetPlayerStatus()!=PMDPlayer::nofile)pplayer->SetPositionInMs(pplayer->GetPositionInMs() + 5000);
	//Up
	if (KeyReleased(KEY_INPUT_UP)) { pplayer->SetVolume(pplayer->GetVolume() + 5); USTR; }
	//Down
	if (KeyReleased(KEY_INPUT_DOWN)) { pplayer->SetVolume(pplayer->GetVolume() - 5); USTR; }
	//P
	if (KeyPressed(KEY_INPUT_P)) { pmdscreen.showVoice = !pmdscreen.showVoice; USTR; }
	//V
	if (KeyPressed(KEY_INPUT_V)) { pmdscreen.showVolume = !pmdscreen.showVolume; USTR; }
	//I
	if (KeyReleased(KEY_INPUT_I))
	{
		char info[1024] = "", singleinfo[1024] = "";
		for (int i = -2; i < 10; i++)
		{
			if (pplayer->GetNotes(singleinfo, i))return;
			strcat(info, singleinfo);
			if (singleinfo[0])strcat(info, "\n");
		}
		TCHAR unicode_str[1024];
		int codepage = CP_ACP;
		while (1)
		{
			MultiByteToWideChar(codepage, 0, info, ARRAYSIZE(info), unicode_str, ARRAYSIZE(unicode_str));
			if (!windowed)strcatDx(unicode_str, LoadLocalString(IDS_DXMSG_APPEND_OK_ENCODING));
			if (windowed ? MessageBox(hWindowDx, unicode_str, LoadLocalString(IDS_MSG_FILE_INFO),
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
				pplayer->SetSSGEffectOn(channelOn[i]);
			else
				pplayer->SetChannelOn(i, channelOn[i]);
		}
	if (KeyReleased(KEY_INPUT_R))
		pplayer->SetRhythmOn(channelOn[10] = !channelOn[10]);
	if (KeyReleased(KEY_INPUT_Z))pplayer->SetPlaybackSpeed(pplayer->GetPlaybackSpeed()*2.0f);
	if (KeyReleased(KEY_INPUT_X))pplayer->SetPlaybackSpeed(1.0f);
	if (KeyReleased(KEY_INPUT_C))pplayer->SetPlaybackSpeed(pplayer->GetPlaybackSpeed()/2.0f);
	if (leftclick.KeyReleased())
	{
		thisclicktime = GetNowCount();
		if (thisclicktime - lastclicktime < mouseDoubleClickTime)
		{
			DPIInfo hdpi;
			SetWindowSize(hdpi.X(originalWinWidth), hdpi.Y(originalWinHeight));
		}
		lastclicktime = thisclicktime;
	}
	if (KeyReleased(KEY_INPUT_M))
		ViewMemEnable(!ViewMemGetEnabled());
	if (KeyReleased(KEY_INPUT_B))
		ChangeBeatsPerBar();
	if (KeyReleased(KEY_INPUT_E))
		ConvertDialog();
	if (pplayer->GetPlayerStatus() == PMDPlayer::playing && pplayer->GetLoopedTimes() == -1)
	{
		pplayer->Unload();
		USTR;
	}
}

void PMDPlay::UpdateString(TCHAR *str, int strsize, bool isplaying, const TCHAR *path)
{
	TCHAR displayPath[MAX_PATH];
	const TCHAR *_strPlayStat = isplaying ? LoadLocalString(IDS_STATUS_PLAYING) : LoadLocalString(IDS_STATUS_IDLE);
	TCHAR strPlayStat[10];
	strcpyDx(strPlayStat, _strPlayStat);
	int mw = displayWinWidth - GetDrawStringWidth(strPlayStat, (int)strlenDx(strPlayStat));
	if (!fileload_ok)
		mw -= GetDrawStringWidth(LoadLocalString(IDS_INVALID_FILE), 6);
	TCHAR strOn[3], strOff[3], strNoLoad[16];
	strcpyDx(strOn, LoadLocalString(IDS_ON));
	strcpyDx(strOff, LoadLocalString(IDS_OFF));
	strcpyDx(strNoLoad, LoadLocalString(IDS_NO_OPEN_FILE));
	snprintfDx(str, strsize, LoadLocalString(IDS_CONTROLS),
		showVoiceAndVolume ? strOn : strOff, pmdscreen.showVoice ? strOn : strOff,
		pmdscreen.showVolume ? strOn : strOff, pplayer->GetVolume(), strPlayStat,
		path[0] ? ShortenPath(path, FALSE, displayPath, GetDefaultFontHandle(), mw) : strNoLoad);
	if (!fileload_ok)strcatDx(str, LoadLocalString(IDS_INVALID_FILE));
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int iShowWindow)
{
	SetThreadUILanguage(GetUserDefaultUILanguage());
	if (lstrcmpi(TEXT("-yr"), lpCmdLine) == 0)
	{
		MessageBox(NULL, TEXT("本程序已经内置节奏声音了，无须释放文件。\nThe rhythm sounds are intergrated to this program, not necessary to do this."), lpCmdLine, MB_ICONINFORMATION);
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
