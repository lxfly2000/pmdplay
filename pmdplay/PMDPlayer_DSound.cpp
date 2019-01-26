#include "PMDPlayer_DSound.h"
#include "pmdmini.h"

#define KEYNAME_NOTIFY_COUNT	"DSoundNotifyCount"
#define KEYNAME_BUFFER_LENGTH	"DSoundBufferTime"
#define VDEFAULT_NOTIFY_COUNT	4
#define VDEFAULT_BUFFER_LENGTH	50

#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dxguid.lib")

#include <dsound.h>

class DSPlayer
{
public:
	void Init(int nChannel, int sampleRate, int bytesPerVar, int onebufbytes);
	void Release();
	void *LockBuffer(DWORD length);
	void UnlockBuffer();
	void Play();
	void Stop(bool resetpos = false);
	void SetVolume(long v);
	long GetVolume();
	HRESULT SetPlaybackSpeed(float);
	void WaitForBufferEndEvent();
private:
	DWORD m_bufbytes, writecursor = 0;
	DWORD lockedBufferBytes;
	void *pLockedBuffer;
	HANDLE hBufferEndEvent;
	IDirectSound8 *pDirectSound;
	IDirectSoundBuffer *pBuffer;
	IDirectSoundNotify *pNotify;
	WAVEFORMATEX w;
};

static DSPlayer x;

void DSPlayer::Init(int nChannel, int sampleRate, int bytesPerVar, int onebufbytes)
{
	w.wFormatTag = WAVE_FORMAT_PCM;
	w.nChannels = nChannel;
	w.nSamplesPerSec = sampleRate;
	w.nAvgBytesPerSec = sampleRate * bytesPerVar*nChannel;
	w.nBlockAlign = 4;
	w.wBitsPerSample = bytesPerVar * 8;
	w.cbSize = 0;

	int notify_count = GetPrivateProfileInt(TEXT(SECTION_NAME), TEXT(KEYNAME_NOTIFY_COUNT), VDEFAULT_NOTIFY_COUNT, TEXT(PROFILE_NAME));
	m_bufbytes = onebufbytes * notify_count;

	C(DirectSoundCreate8(NULL, &pDirectSound, NULL));
	C(pDirectSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_NORMAL));
	DSBUFFERDESC desc = { sizeof desc,DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY |
		DSBCAPS_GLOBALFOCUS,m_bufbytes,0,&w,DS3DALG_DEFAULT };
	C(pDirectSound->CreateSoundBuffer(&desc, &pBuffer, NULL));
	C(pBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&pNotify));
	DSBPOSITIONNOTIFY *dpn = new DSBPOSITIONNOTIFY[notify_count];
	hBufferEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	for (int i = 0; i < notify_count; i++)
	{
		dpn[i].dwOffset = m_bufbytes * i / notify_count;
		dpn[i].hEventNotify = hBufferEndEvent;
	}
	C(pNotify->SetNotificationPositions(notify_count, dpn));
	delete[]dpn;
}

void DSPlayer::Release()
{
	CloseHandle(hBufferEndEvent);
	pBuffer->Release();
	pDirectSound->Release();
}

void *DSPlayer::LockBuffer(DWORD length)
{
	C(pBuffer->Lock(writecursor, length, &pLockedBuffer, &lockedBufferBytes, NULL, NULL, NULL));
	return pLockedBuffer;
}

void DSPlayer::UnlockBuffer()
{
	C(pBuffer->Unlock(pLockedBuffer, lockedBufferBytes, NULL, NULL));
	writecursor = (writecursor + lockedBufferBytes) % m_bufbytes;
}

void DSPlayer::Play()
{
	C(pBuffer->Play(0, 0, DSBPLAY_LOOPING));
}

void DSPlayer::Stop(bool resetpos)
{
	C(pBuffer->Stop());
	if (resetpos)
	{
		//因为播放时有两个已渲染缓冲区且停止时有一个未播放所以需要重置位置。（仅限DSound）
		C(pBuffer->SetCurrentPosition(0));
		writecursor = 0;
	}
}

void DSPlayer::SetVolume(long v)
{
	C(pBuffer->SetVolume(v));
}

long DSPlayer::GetVolume()
{
	long v;
	C(pBuffer->GetVolume(&v));
	return v;
}

void DSPlayer::WaitForBufferEndEvent()
{
	WaitForSingleObject(hBufferEndEvent, INFINITE);
}

HRESULT DSPlayer::SetPlaybackSpeed(float speed)
{
	return pBuffer->SetFrequency((DWORD)(w.nSamplesPerSec*speed));
}

int PMDPlayer_DSound::Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)
{
	if (PMDPlayer::Init(nChannel, sampleRate, bytesPerVar, buffer_time_ms))
		return -1;
	if (buffer_time_ms == 0)
		buffer_time_ms = GetPrivateProfileInt(TEXT(SECTION_NAME), TEXT(KEYNAME_BUFFER_LENGTH), VDEFAULT_BUFFER_LENGTH, TEXT(PROFILE_NAME));
	bytesof_soundbuffer = sampleRate * bytesPerVar*nChannel*buffer_time_ms / 1000;
	x.Init(nChannel, sampleRate, bytesPerVar, bytesof_soundbuffer);
	return 0;
}

int PMDPlayer_DSound::Release()
{
	if (PMDPlayer::Release())
		return -1;
	x.Release();
	return 0;
}

int PMDPlayer_DSound::Play()
{
	x.Play();
	if (PMDPlayer::Play())
		return -1;
	return 0;
}

int PMDPlayer_DSound::Pause()
{
	if (PMDPlayer::Pause())
		return -1;
	x.Stop();
	return 0;
}

int PMDPlayer_DSound::Stop()
{
	if (PMDPlayer::Stop())
		return -1;
	x.Stop(true);
	return 0;
}

int PMDPlayer_DSound::SetPlaybackSpeed(float speed)
{
	if (x.SetPlaybackSpeed(playbackspeed = speed) == S_OK)
		return 0;
	else
		return -1;
}

void PMDPlayer_DSound::SetVolume(int v)
{
	v = min(PMDPLAYER_MAX_VOLUME, v);
	v = max(0, v);
	x.SetVolume(v*(DSBVOLUME_MAX - DSBVOLUME_MIN) / PMDPLAYER_MAX_VOLUME - (DSBVOLUME_MAX - DSBVOLUME_MIN));
}

int PMDPlayer_DSound::GetVolume()
{
	return (x.GetVolume() - DSBVOLUME_MIN) / ((DSBVOLUME_MAX - DSBVOLUME_MIN) / PMDPLAYER_MAX_VOLUME);
}

void PMDPlayer_DSound::OnPlay()
{
	pmd_renderer((short*)x.LockBuffer(bytesof_soundbuffer), bytesof_soundbuffer / m_channels / m_bytesPerVar);
	x.UnlockBuffer();
	x.WaitForBufferEndEvent();
	PMDPlayer::OnPlay();
}
