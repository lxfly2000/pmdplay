#include<fstream>
#include<string>
#include "PMDPlayer.h"
#include "../shared/pmdmini.h"
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dxguid.lib")

#ifdef _DEBUG
#define C(e) if(e)\
if(MessageBox(NULL,__FILE__ L":" _STRINGIZE(__LINE__) "\n" __FUNCTION__ "\n" _STRINGIZE(e),NULL,MB_OKCANCEL)==IDOK)\
DebugBreak()
#else
#define C(e) e
#endif

void XAPlayer::Init(int nChannel, int sampleRate, int bytesPerVar, int onebufbytes)
{
	w.wFormatTag = WAVE_FORMAT_PCM;
	w.nChannels = nChannel;
	w.nSamplesPerSec = sampleRate;
	w.nAvgBytesPerSec = sampleRate*bytesPerVar*nChannel;
	w.nBlockAlign = 4;
	w.wBitsPerSample = bytesPerVar * 8;
	w.cbSize = 0;

	int notify_count = GetPrivateProfileInt(sectionname, varstring_notifycount, 4, profilename);
	m_bufbytes = onebufbytes*notify_count;

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
		dpn[i].dwOffset = m_bufbytes*i / notify_count;
		dpn[i].hEventNotify = hBufferEndEvent;
	}
	C(pNotify->SetNotificationPositions(notify_count, dpn));
	delete[]dpn;
}

void XAPlayer::Release()
{
	CloseHandle(hBufferEndEvent);
	pBuffer->Release();
	pDirectSound->Release();
}

void *XAPlayer::LockBuffer(DWORD length)
{
	C(pBuffer->Lock(writecursor, length, &pLockedBuffer, &lockedBufferBytes, NULL, NULL, NULL));
	return pLockedBuffer;
}

void XAPlayer::UnlockBuffer()
{
	C(pBuffer->Unlock(pLockedBuffer, lockedBufferBytes, NULL, NULL));
	writecursor = (writecursor + lockedBufferBytes) % m_bufbytes;
}

void XAPlayer::Play()
{
	C(pBuffer->Play(0, 0, DSBPLAY_LOOPING));
}

void XAPlayer::Stop(bool resetpos)
{
	C(pBuffer->Stop());
	if (resetpos)
	{
		//因为播放时有两个已渲染缓冲区且停止时有一个未播放所以需要重置位置。（仅限DSound）
		C(pBuffer->SetCurrentPosition(0));
		writecursor = 0;
	}
}

void XAPlayer::SetVolume(long v)
{
	C(pBuffer->SetVolume(v));
}

long XAPlayer::GetVolume()
{
	long v;
	C(pBuffer->GetVolume(&v));
	return v;
}

void XAPlayer::WaitForBufferEndEvent()
{
	WaitForSingleObject(hBufferEndEvent, INFINITE);
}

HRESULT XAPlayer::SetPlaybackSpeed(float speed)
{
	return pBuffer->SetFrequency((DWORD)(w.nSamplesPerSec*speed));
}


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

bool PMDPlayer::LoadRhythmFromDirectory(char* dir)
{
	return loadrhythmsample(dir);
}

bool PMDPlayer::LoadRhythmFromMemory(char* bd, char* sd, char* top, char* hh, char* tom, char* rim)
{
	return loadrhythmsample_mem(bd, sd, top, hh, tom, rim);
}

bool PMDPlayer::Convert(char *srcfile, char *outfile, int loops, int fadetime, bool splittracks)
{
	struct WaveStructure
	{
		char strRIFF[4];
		int chunkSize;
		char strFormat[4];
		char strFmt[4];
		int subchunk1Size;
		short audioFormat;
		short numChannels;
		int sampleRate;
		int byteRate;
		short blockAlign;
		short bpsample;//Bits per sample
		char strData[4];
		int subchunk2Size;//Data size（字节数）
	};
	WaveStructure wavfileheader=
	{
		'R','I','F','F',//strRIFF
		0,//chunkSize
		'W','A','V','E',//strFormat
		'f','m','t',' ',//strFmt
		16,//subchunk1Size
		WAVE_FORMAT_PCM,//audioFormat
		(short)m_channels,//numChannels
		m_sampleRate,//sampleRate
		m_sampleRate*m_channels*m_bytesPerVar,//byteRate
		(short)(m_channels*m_bytesPerVar),//blockAlign
		(short)(m_bytesPerVar*8),//bpsample
		'd','a','t','a',//strData
		0//subchunk2Size
	};
	short *soundbuffer = (short*)new char[bytesof_soundbuffer];
	if (LoadFromFile(srcfile))return false;
	if (splittracks)
	{
		char mtname[MAX_PATH];
		for(int i=0;i<10;i++)
		{
			wavfileheader.subchunk2Size = 0;
			sprintf(mtname, "%s-%c.wav", outfile, i == 9 ? 'R' : 'A' + i);
			for (int j = 0; j < 10; j++)
			{
				if (j == 9)
					setrhythmwithssgeffect(i == 9);
				else if (j == 8)
					getopenwork()->effflag = (i != 8);
				else
					j == i ? maskoff(j) : maskon(j);
			}
			setpos2(0);
			std::fstream f(mtname, std::ios::out | std::ios::binary);
			f.seekp(sizeof wavfileheader);
			int currentLoops = GetLoopedTimes();
			while (currentLoops < loops && currentLoops >= 0)
			{
				pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
				f.write((char*)soundbuffer, bytesof_soundbuffer);
				wavfileheader.subchunk2Size += bytesof_soundbuffer;
				currentLoops = GetLoopedTimes();
			}
			fadingout_end_time_sec = (GetPositionInMs() + fadetime) / 1000;
			if (currentLoops >= 0)fadeout2(fadetime);
			while (GetPositionInMs() / 1000 < fadingout_end_time_sec)
			{
				pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
				f.write((char*)soundbuffer, bytesof_soundbuffer);
				wavfileheader.subchunk2Size += bytesof_soundbuffer;
			}
			wavfileheader.chunkSize = 36 + wavfileheader.subchunk2Size;
			f.seekp(0);
			f.write((char*)&wavfileheader, sizeof wavfileheader);
		}
	}
	else
	{
		std::fstream f(outfile, std::ios::out | std::ios::binary);
		f.seekp(sizeof wavfileheader);
		int currentLoops = GetLoopedTimes();
		while (currentLoops < loops && currentLoops >= 0)
		{
			pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
			f.write((char*)soundbuffer, bytesof_soundbuffer);
			wavfileheader.subchunk2Size += bytesof_soundbuffer;
			currentLoops = GetLoopedTimes();
		}
		fadingout_end_time_sec = (GetPositionInMs() + fadetime) / 1000;
		if (currentLoops >= 0)fadeout2(fadetime);
		while (GetPositionInMs() / 1000 < fadingout_end_time_sec)
		{
			pmd_renderer(soundbuffer, bytesof_soundbuffer / m_channels / m_bytesPerVar);
			f.write((char*)soundbuffer, bytesof_soundbuffer);
			wavfileheader.subchunk2Size += bytesof_soundbuffer;
		}
		wavfileheader.chunkSize = 36 + wavfileheader.subchunk2Size;
		f.seekp(0);
		f.write((char*)&wavfileheader, sizeof wavfileheader);
	}
	Unload();
	delete[]soundbuffer;
	return true;
}

int PMDPlayer::Play()
{
	if (playerstatus != paused)return -1;
	playerstatus = playing;
	x.Play();
	tSubPlayback = std::thread(PMDPlayer::_Subthread_Playback, this);
	return 0;
}

int PMDPlayer::Pause()
{
	if (playerstatus != nofile && playerstatus != playing)return -1;
	playerstatus = paused;
	tSubPlayback.join();
	x.Stop();
	return 0;
}

HRESULT PMDPlayer::SetPlaybackSpeed(float speed)
{
	return x.SetPlaybackSpeed(playbackspeed = speed);
}

float PMDPlayer::GetPlaybackSpeed()
{
	return playbackspeed;
}

void PMDPlayer::SetVolume(int v)
{
	v = min(PMDPLAYER_MAX_VOLUME, v);
	v = max(0, v);
	x.SetVolume(v*(DSBVOLUME_MAX - DSBVOLUME_MIN) / PMDPLAYER_MAX_VOLUME - (DSBVOLUME_MAX - DSBVOLUME_MIN));
}

int PMDPlayer::GetVolume()
{
	return (x.GetVolume() - DSBVOLUME_MIN) / ((DSBVOLUME_MAX - DSBVOLUME_MIN) / PMDPLAYER_MAX_VOLUME);
}

void PMDPlayer::Unload()
{
	playerstatus = nofile;
	if (tSubPlayback.joinable())tSubPlayback.join();
	x.Stop(true);
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
	m_sampleRate = sampleRate;
	bytesof_soundbuffer = sampleRate*bytesPerVar*nChannel*buffer_time_ms / 1000;
	x.Init(nChannel, sampleRate, bytesPerVar, bytesof_soundbuffer);
	playerstatus = nofile;
	pmd_init();
	setppsuse(false);
	setrhythmwithssgeffect(true);
	getopenwork()->effflag = 0;
	setfmcalc55k(true);
	pmd_setrate(sampleRate);
	playbackspeed = 1.0f;
}

void PMDPlayer::Release()
{
	Unload();
	x.Release();
}

void PMDPlayer::_Subthread_Playback(PMDPlayer *param)
{
	param->_LoopPlayback();
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
	pmd_renderer((short*)x.LockBuffer(bytesof_soundbuffer), bytesof_soundbuffer / m_channels / m_bytesPerVar);
	x.UnlockBuffer();
	x.WaitForBufferEndEvent();
	for (int i = 0; i < ARRAYSIZE(keyState); i++)
	{
		//*(unsigned short*)(keyState + i) = (getpartwork(i)->onkai & 0xF) + ((getpartwork(i)->onkai >> 4) * 12);
		//((unsigned char*)(keyState + i))[3] = getpartwork(i)->voicenum;
		//((unsigned char*)(keyState + i))[4] = getpartwork(i)->volume;
		//虽然是整型的，但该变量只用了一个字节，低四位表示一个八度内的半音（Semitone，https://zh.wikipedia.org/wiki/半音 ），
		//高四位表示在哪个八度（Octave，https://zh.wikipedia.org/wiki/純八度 ），因此实际的音高是低四位＋高四位×12.
		//这个问题害得我折腾了一下午……原作者也不在注释上写明白，妈的法克！！(╯‵□′)╯︵┻━┻
		keyState[i] = (getpartwork(i)->onkai & 0xF) + ((getpartwork(i)->onkai >> 4) * 12);
		if (keyState[i] == 195)keyState[i] = -1;
		voiceState[i] = getpartwork(i)->voicenum;
		volumeState[i] = getpartwork(i)->volume;
	}
	if (!getopenwork()->effflag)keyState[8] = getopenwork()->kshot_dat;// % 128;//SSG鼓声
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