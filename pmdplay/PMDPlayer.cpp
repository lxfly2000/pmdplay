#include "PMDPlayer.h"
#include "pmdmini.h"
#include "pmdwin/pmdwin.h"
#include<fstream>
#include<string>



int PMDPlayer::LoadFromFile(const char *filepath)
{
	if (!pmd_is_pmd(filepath))return -1;
	getlength((char*)filepath, (int*)&length_in_ms, (int*)&loop_in_ms);
	//http://blog.csdn.net/tulip527/article/details/7976471
	std::fstream f(filepath, std::ios::binary | std::ios::in);
	std::string sf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return LoadFromMemory((uchar*)sf.data(), (int)sf.size());
}

int PMDPlayer::LoadFromMemory(unsigned char *pdata, int length)
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
		const char mtchannels[] = "ABCDEFGHIKR";
		for(int i=0;i<11;i++)
		{
			wavfileheader.subchunk2Size = 0;
			sprintf(mtname, "%s-%c.wav", outfile, mtchannels[i]);
			for (int j = 0; j < 11; j++)
			{
				if (j == 10)
					setrhythmwithssgeffect(i == 10);
				else if (j == 9)
					getopenwork()->effflag = (i != 9);
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
		if (!f)
		{
			Unload();
			delete[]soundbuffer;
			return false;
		}
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
	tSubPlayback = std::thread(PMDPlayer::_Subthread_Playback, this);
	return 0;
}

int PMDPlayer::Pause()
{
	if (playerstatus != nofile && playerstatus != playing)return -1;
	playerstatus = paused;
	tSubPlayback.join();
	return 0;
}

int PMDPlayer::SetPlaybackSpeed(float speed)
{
	return -1;
}

float PMDPlayer::GetPlaybackSpeed()
{
	return playbackspeed;
}

void PMDPlayer::SetVolume(int v)
{
}

int PMDPlayer::GetVolume()
{
	return 0;
}

void PMDPlayer::Unload()
{
	playerstatus = nofile;
	if (tSubPlayback.joinable())tSubPlayback.join();
	pmd_stop();
	Stop();
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

int PMDPlayer::Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)
{
	m_channels = nChannel;
	m_bytesPerVar = bytesPerVar;
	m_sampleRate = sampleRate;
	playerstatus = nofile;
	pmd_init();
	setppsuse(false);
	setrhythmwithssgeffect(true);
	getopenwork()->effflag = 0;
	setfmcalc55k(true);
	pmd_setrate(sampleRate);
	playbackspeed = 1.0f;
	memset(keyState, -1, sizeof keyState);
	ZeroMemory(voiceState, sizeof voiceState);
	ZeroMemory(volumeState, sizeof volumeState);
	return 0;
}

int PMDPlayer::Release()
{
	Unload();
	return 0;
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
void PMDPlayer::OnPlay()
{
	for (int i = 0; i < ARRAYSIZE(keyState); i++)
	{
		//*(unsigned short*)(keyState + i) = (getpartwork(i)->onkai & 0xF) + ((getpartwork(i)->onkai >> 4) * 12);
		//((unsigned char*)(keyState + i))[3] = getpartwork(i)->voicenum;
		//((unsigned char*)(keyState + i))[4] = getpartwork(i)->volume;
		//虽然是整型的，但该变量只用了一个字节，低四位表示一个八度内的半音（Semitone，https://zh.wikipedia.org/wiki/半音 ），
		//高四位表示在哪个八度（Octave，https://zh.wikipedia.org/wiki/純八度 ），因此实际的音高是低四位＋高四位×12.
		//这个问题害得我折腾了一下午……原作者也不在注释上写明白，妈的法克！！(╯‵□′)╯︵┻━┻
		if (getpartwork(i)->onkai == 255||getpartwork(i)->keyoff_flag==-1)
			keyState[i] = -1;
		else
			keyState[i] = (getpartwork(i)->onkai & 0xF) + ((getpartwork(i)->onkai >> 4) * 12);
		voiceState[i] = getpartwork(i)->voicenum;
		volumeState[i] = getpartwork(i)->volume;
	}
	if (!getopenwork()->effflag)
	{
		int tmpk = getopenwork()->kshot_dat;
		if (tmpk)
			keyState[8] = tmpk;// % 128;//SSG鼓声
	}
}

void PMDPlayer::OnFadingOut()
{
	OnPlay();
	if (GetPositionInMs() / 1000 > fadingout_end_time_sec)
		playerstatus = fadedout;
}

void PMDPlayer::SetPositionInMs(int pos)
{
	setpos(pos);
}

void PMDPlayer::SetChannelOn(int n, bool on)
{
	on ? maskoff(n) : maskon(n);
}

void PMDPlayer::SetSSGEffectOn(bool on)
{
	getopenwork()->effflag = !on;
}

bool PMDPlayer::GetSSGEffectOn()
{
	return !getopenwork()->effflag;
}

void PMDPlayer::SetRhythmOn(bool on)
{
	setrhythmwithssgeffect(on);
}

int PMDPlayer::Stop()
{
	return 0;
}
