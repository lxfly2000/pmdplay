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

bool PMDPlayer::subthread_on = false;

int PMDPlayer::LoadFromFile(const char *filepath)
{
	if (!pmd_is_pmd(filepath))return -1;
	int length_in_ms, loop_in_ms;
	getlength((char*)filepath, &length_in_ms, &loop_in_ms);
	length_in_sec = length_in_ms / 1000;
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
	played_buffers = position_in_sec = 0;
	return r;
}

int PMDPlayer::Play()
{
	if (playerstatus != paused)return -1;
	playerstatus = playing;
	_beginthread(PMDPlayer::_Subthread_Playback, 0, this);
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
	pmd_stop();
	played_buffers = position_in_sec = 0;
}

int PMDPlayer::FadeoutAndStop(int time_ms)
{
	if (playerstatus != playing)return -1;
	fadingout_end_time_sec = position_in_sec + time_ms / 1000;
	fadeout2(time_ms);
	playerstatus = fadingout;
	return 0;
}

const int *PMDPlayer::GetKeysState()
{
	return keyState;
}

int PMDPlayer::GetNotes(char *outstr, int al)
{
	if (playerstatus == nofile)return -1;
	getmemo(outstr, pSourceData, lengthSourceData, al);
	return 0;
}

unsigned PMDPlayer::GetLengthInSec()
{
	return length_in_sec;
}

unsigned PMDPlayer::GetPositionInSec()
{
	return position_in_sec;
}

int PMDPlayer::GetLoopedTimes()
{
	return getloopcount();
}

PMDPlayer::PlayerStatus PMDPlayer::GetPlayerStatus()
{
	return playerstatus;
}

void PMDPlayer::Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)
{
	m_channels = nChannel;
	m_bytesPerVar = bytesPerVar;
	m_bytesPerSample = m_channels*m_bytesPerVar;
	m_byteRate = sampleRate*m_bytesPerVar*m_channels;
	bytesof_soundbuffer = m_byteRate*buffer_time_ms / 1000;
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
	while (subthread_on);
	delete[]soundbuffer;
}

void PMDPlayer::_Subthread_Playback(void *param)
{
	subthread_on = true;
	((PMDPlayer*)param)->_LoopPlayback();
	subthread_on = false;
	_endthread();
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

void PMDPlayer::OnPlay()
{
	pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
	x.Play((BYTE*)soundbuffer, bytesof_soundbuffer);
	while (x.GetQueuedBuffersNum());
	for (int i = 0; i < ARRAYSIZE(keyState); i++)
		keyState[i] = getopenwork()->MusPart[i]->onkai;
	played_buffers++;
	position_in_sec = played_buffers * bytesof_soundbuffer / m_byteRate;
}

void PMDPlayer::OnFadingOut()
{
	OnPlay();
	if (position_in_sec > fadingout_end_time_sec)
		Unload();
}