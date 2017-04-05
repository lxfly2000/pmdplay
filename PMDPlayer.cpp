#include<process.h>
#include<fstream>
#include<string>
#include "PMDPlayer.h"
#include "pmdmini.h"
XAPlayer::XAPlayer(int nChannel, int sampleRate, int bytesPerVar)
{
	Init(nChannel, sampleRate, bytesPerVar);
}

void XAPlayer::Init(int nChannel, int sampleRate, int bytesPerVar)
{
	xAudio2Engine = NULL;
	masterVoice = NULL;
	sourceVoice = NULL;
	WAVEFORMATEX w;
	w.wFormatTag = WAVE_FORMAT_PCM;
	w.nChannels = nChannel;
	w.nSamplesPerSec = sampleRate;
	w.nAvgBytesPerSec = sampleRate*bytesPerVar*nChannel;
	w.nBlockAlign = 4;
	w.wBitsPerSample = bytesPerVar * 8;
	w.cbSize = 0;
	xbuffer = { 0 };
	xbuffer.Flags = XAUDIO2_END_OF_STREAM;
	if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))return;
	if (FAILED(XAudio2Create(&xAudio2Engine)))return;
	if (FAILED(xAudio2Engine->CreateMasteringVoice(&masterVoice)))return;
	xAudio2Engine->CreateSourceVoice(&sourceVoice, &w);
}

XAPlayer::~XAPlayer()
{
	Release();
}

void XAPlayer::Release()
{
	sourceVoice->Stop();
	sourceVoice->FlushSourceBuffers();
	if (sourceVoice)sourceVoice->DestroyVoice();
	masterVoice->DestroyVoice();
	xAudio2Engine->Release();
	CoUninitialize();
}

void XAPlayer::Play(BYTE*buf, int length)
{
	xbuffer.pAudioData = buf;
	xbuffer.AudioBytes = length;
	if (SUCCEEDED(sourceVoice->SubmitSourceBuffer(&xbuffer)))
		sourceVoice->Start();
}

int XAPlayer::GetQueuedBuffersNum()
{
	sourceVoice->GetState(&state);
	return state.BuffersQueued;
}


PMDPlayer::PMDPlayer(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms) :x(nChannel, sampleRate, bytesPerVar)
{
	Init(nChannel, sampleRate, bytesPerVar, buffer_time_ms);
}

PMDPlayer::~PMDPlayer()
{
	Release();
}

HANDLE PMDPlayer::hSubPlayback = NULL;

int PMDPlayer::LoadFromFile(const char *filepath)
{
	if (!pmd_is_pmd(filepath))return -1;
	getlength((char*)filepath, (int*)&length_in_ms, (int*)&loop_in_ms);
	//http://blog.csdn.net/tulip527/article/details/7976471
	std::fstream f(filepath, std::ios::binary | std::ios::in);
	std::string sf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return LoadFromMemory((uchar*)sf.data(), (int)sf.size());
}

int PMDPlayer::LoadFromMemory(uchar *pdata, int length)
{
	char curdir[MAX_PATH];
	char* path[2] = { curdir,NULL };
	Unload();
	GetCurrentDirectoryA(MAX_PATH, curdir);
	setpcmdir(path);
	lengthSourceData = length;
	playerstatus = paused;
	int r = music_load2(pdata, length);
	music_start();
	pSourceData = getopenwork()->mmlbuf - 1;
	return r;
}

int PMDPlayer::Play()
{
	if (playerstatus != paused)return -1;
	playerstatus = playing;
	hSubPlayback = (HANDLE)_beginthreadex(NULL, 0, PMDPlayer::_Subthread_Playback, this, NULL, NULL);
	return 0;
}

int PMDPlayer::Pause()
{
	if (playerstatus != nofile && playerstatus != playing)return -1;
	playerstatus = paused;
	return 0;
}

void PMDPlayer::Unload()
{
	playerstatus = nofile;
	WaitForSingleObject(hSubPlayback, INFINITE);
	if (hSubPlayback)
	{
		CloseHandle(hSubPlayback);
		hSubPlayback = NULL;
	}
	pmd_stop();
}

int PMDPlayer::FadeoutAndStop(int time_ms)
{
	if (playerstatus != playing)return -1;
	fadingout_end_time_sec = (GetPositionInMs() + time_ms) / 1000;
	fadeout2(time_ms);
	playerstatus = fadingout;
	return 0;
}

const int *PMDPlayer::GetKeysState()
{
	return keyState;
}

const int *PMDPlayer::GetKeyVoice()
{
	return voiceState;
}

const int *PMDPlayer::GetKeyVolume()
{
	return volumeState;
}

int PMDPlayer::GetNotes(char *outstr, int al)
{
	if (playerstatus == nofile)return -1;
	getmemo(outstr, pSourceData, lengthSourceData, al);
	return 0;
}

unsigned PMDPlayer::GetLengthInMs()
{
	return length_in_ms;
}

unsigned PMDPlayer::GetPositionInMs()
{
	return getpos();
}

unsigned PMDPlayer::GetLoopLengthInMs()
{
	return loop_in_ms;
}

int PMDPlayer::GetLoopedTimes()
{
	return getloopcount();
}

int PMDPlayer::GetTempo()
{
	return getopenwork()->tempo_48 * 2;
}

int PMDPlayer::GetPositionInCount()
{
	return getpos2();
}

int PMDPlayer::GetXiaojieLength()
{
	return getopenwork()->syousetu_lng;
}

PMDPlayer::PlayerStatus PMDPlayer::GetPlayerStatus()
{
	return playerstatus;
}

void PMDPlayer::Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)
{
	m_channels = nChannel;
	m_bytesPerVar = bytesPerVar;
	bytesof_soundbuffer = sampleRate*bytesPerVar*nChannel*buffer_time_ms / 1000;
	soundbuffer = reinterpret_cast<decltype(soundbuffer)>(new BYTE[bytesof_soundbuffer]);
	playerstatus = nofile;
	pmd_init();
	setppsuse(false);
	setrhythmwithssgeffect(true);
	getopenwork()->effflag = 0;
	setfmcalc55k(true);
	pmd_setrate(sampleRate);
}

void PMDPlayer::Release()
{
	Unload();
	delete[]soundbuffer;
}

unsigned WINAPI PMDPlayer::_Subthread_Playback(void *param)
{
	((PMDPlayer*)param)->_LoopPlayback();
	return 0;
}

void PMDPlayer::_LoopPlayback()
{
	while (playerstatus >= playing)
	{
		switch (playerstatus)
		{
		case playing:OnPlay(); break;
		case fadingout:OnFadingOut(); break;
		}
	}
}
//#define NOTE_PITCH_NOT_CORRECT
#ifdef NOTE_PITCH_NOT_CORRECT
#include<DxLib.h>
int look = 0;
TCHAR *des[24] = {
	L"FM1",L"FM2",L"FM3",L"FM4",L"FM5",L"FM6",
	L"SSG1",L"SSG2",L"SSG3",
	L"ADPCM",
	L"OPNAR",
	L"Ext1",L"Ext2",L"Ext3",
	L"Rhy",
	L"Eff",
	L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ"
};
#endif
void PMDPlayer::OnPlay()
{
	pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
	x.Play((BYTE*)soundbuffer, bytesof_soundbuffer);
	while (x.GetQueuedBuffersNum());
	for (int i = 0; i < ARRAYSIZE(keyState); i++)
	{
		//*(unsigned short*)(keyState + i) = (getpartwork(i)->onkai & 0xF) + ((getpartwork(i)->onkai >> 4) * 12);
		//((unsigned char*)(keyState + i))[3] = getpartwork(i)->voicenum;
		//((unsigned char*)(keyState + i))[4] = getpartwork(i)->volume;
		//虽然是整型的，但该变量只用了一个字节，低四位表示一个八度内的半音（Semitone，https://zh.wikipedia.org/wiki/半音 ），
		//高四位表示在哪个八度（Octave，https://zh.wikipedia.org/wiki/純八度 ），因此实际的音高是低四位＋高四位×12.
		//这个问题害得我折腾了一下午……原作者也不在注释上写明白，妈的法克！！(╯‵□′)╯︵┻━┻
		keyState[i] = (getpartwork(i)->onkai & 0xF) + ((getpartwork(i)->onkai >> 4) * 12);
		voiceState[i] = getpartwork(i)->voicenum;
		volumeState[i] = getpartwork(i)->volume;
	}
	if (!getopenwork()->effflag)keyState[8] = getopenwork()->kshot_dat % 128;//SSG鼓声
#ifdef NOTE_PITCH_NOT_CORRECT
	if (GetAsyncKeyState(VK_UP)&1)look++;
	if (GetAsyncKeyState(VK_DOWN)&1)look--;
	clsDx();
	if (look == -1)
	{
		for (int i = 0; i < 524; i++)
		{
			if (i % 16 == 0)printfDx(TEXT("\n"));
			printfDx(TEXT("%02x "), ((char*)getopenwork())[i] & 0xFF);
		}
	}
	else
	{
		printfDx(TEXT("%d %s"), look, des[look]);
		for (int i = 0; i < sizeof QQ; i++)
		{
			if (i % 16 == 0)printfDx(TEXT("\n"));
			printfDx(TEXT("%02x "), ((char*)getpartwork(look))[i] & 0xFF);
		}
	}
#endif
}

void PMDPlayer::OnFadingOut()
{
	OnPlay();
	if (GetPositionInMs() / 1000 > fadingout_end_time_sec)
		playerstatus = fadedout;
}