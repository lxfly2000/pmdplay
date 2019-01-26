#include "PMDPlayer_XAudio2.h"
#include "pmdmini.h"

#pragma comment(lib,"XAudio2.lib")

#define KEYNAME_BUFFER_LENGTH	"XAudio2BufferTime"
#define VDEFAULT_BUFFER_LENGTH	20

#include <xaudio2.h>

class XASCallback :public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;
	XASCallback();
	~XASCallback();
	void WINAPI OnBufferEnd(void *)override;
	void WINAPI OnBufferStart(void*)override {}
	void WINAPI OnLoopEnd(void*)override {}
	void WINAPI OnStreamEnd()override {}
	void WINAPI OnVoiceError(void*, HRESULT)override {}
	void WINAPI OnVoiceProcessingPassEnd()override {}
	void WINAPI OnVoiceProcessingPassStart(UINT32)override {}
};
//https://github.com/lxfly2000/XAPlayer
class XAPlayer
{
public:
	int Init(int nChannel, int sampleRate, int bytesPerVar);
	void Release();
	void Play(BYTE* buf, int length);
	void SetVolume(float v);
	float GetVolume();
	int SetPlaybackSpeed(float);
	int GetQueuedBuffersNum();
	void WaitForBufferEndEvent();
private:
	IXAudio2*xAudio2Engine;
	IXAudio2MasteringVoice* masterVoice;
	IXAudio2SourceVoice* sourceVoice;
	XAUDIO2_BUFFER xbuffer;
	XAUDIO2_VOICE_STATE state;
	XASCallback xcallback;
};

static XAPlayer x;
static short* soundbuffer;//PMD_Renderer那个函数用的类型是short我表示难以理解……

XASCallback::XASCallback()
{
	hBufferEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

XASCallback::~XASCallback()
{
	CloseHandle(hBufferEndEvent);
}

void WINAPI XASCallback::OnBufferEnd(void *p)
{
	SetEvent(hBufferEndEvent);
}

int XAPlayer::Init(int nChannel, int sampleRate, int bytesPerVar)
{
	xAudio2Engine = NULL;
	masterVoice = NULL;
	sourceVoice = NULL;
	WAVEFORMATEX w;
	w.wFormatTag = WAVE_FORMAT_PCM;
	w.nChannels = nChannel;
	w.nSamplesPerSec = sampleRate;
	w.nAvgBytesPerSec = sampleRate * bytesPerVar*nChannel;
	w.nBlockAlign = 4;
	w.wBitsPerSample = bytesPerVar * 8;
	w.cbSize = 0;
	xbuffer = { 0 };
	xbuffer.Flags = XAUDIO2_END_OF_STREAM;
	if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
		return -1;
	if (FAILED(XAudio2Create(&xAudio2Engine)))
		return -2;
	if (FAILED(xAudio2Engine->CreateMasteringVoice(&masterVoice)))
		return -3;
	if (FAILED(xAudio2Engine->CreateSourceVoice(&sourceVoice, &w, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &xcallback)))
		return -4;
	if (FAILED(sourceVoice->Start()))
		return -5;
	return 0;
}

void XAPlayer::Release()
{
	if (sourceVoice)
	{
		sourceVoice->Stop();
		sourceVoice->FlushSourceBuffers();
		sourceVoice->DestroyVoice();
	}
	if (masterVoice)
		masterVoice->DestroyVoice();
	if (xAudio2Engine)
		xAudio2Engine->Release();
	CoUninitialize();
}

void XAPlayer::Play(BYTE*buf, int length)
{
	xbuffer.pAudioData = buf;
	xbuffer.AudioBytes = length;
	sourceVoice->SubmitSourceBuffer(&xbuffer);
}

void XAPlayer::SetVolume(float v)
{
	masterVoice->SetVolume(v);
}

float XAPlayer::GetVolume()
{
	float v;
	masterVoice->GetVolume(&v);
	return v;
}

int XAPlayer::GetQueuedBuffersNum()
{
	sourceVoice->GetState(&state);
	return state.BuffersQueued;
}

void XAPlayer::WaitForBufferEndEvent()
{
	WaitForSingleObject(xcallback.hBufferEndEvent, INFINITE);
}

int XAPlayer::SetPlaybackSpeed(float speed)
{
	return sourceVoice->SetFrequencyRatio(speed);
}


int PMDPlayer_XAudio2::Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)
{
	if (PMDPlayer::Init(nChannel, sampleRate, bytesPerVar, buffer_time_ms))
		return -1;
	if (x.Init(nChannel, sampleRate, bytesPerVar))
		return -2;
	if (buffer_time_ms == 0)
		buffer_time_ms = GetPrivateProfileInt(TEXT(SECTION_NAME), TEXT(KEYNAME_BUFFER_LENGTH), VDEFAULT_BUFFER_LENGTH, TEXT(PROFILE_NAME));
	bytesof_soundbuffer = sampleRate*bytesPerVar*nChannel*buffer_time_ms / 1000;
	soundbuffer = reinterpret_cast<decltype(soundbuffer)>(new BYTE[bytesof_soundbuffer]);
	return 0;
}

int PMDPlayer_XAudio2::Release()
{
	if (PMDPlayer::Release())
		return -1;
	delete[]soundbuffer;
	x.Release();
	return 0;
}

int PMDPlayer_XAudio2::SetPlaybackSpeed(float speed)
{
	return x.SetPlaybackSpeed(playbackspeed = speed);
}

void PMDPlayer_XAudio2::SetVolume(int v)
{
	v = min(PMDPLAYER_MAX_VOLUME, v);
	v = max(0, v);
	x.SetVolume((float)v / PMDPLAYER_MAX_VOLUME);
}

int PMDPlayer_XAudio2::GetVolume()
{
	return (int)(x.GetVolume()*PMDPLAYER_MAX_VOLUME);
}

void PMDPlayer_XAudio2::OnPlay()
{
	pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
	x.Play((BYTE*)soundbuffer, bytesof_soundbuffer);
	x.WaitForBufferEndEvent();
	PMDPlayer::OnPlay();
}
