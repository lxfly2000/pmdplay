#pragma once
//https://github.com/mistydemeo/pmdmini
#include <dsound.h>
#include <thread>
#include "pmdwin\pmdwinimport.h"

#define PMDPLAYER_MAX_VOLUME	100
const TCHAR sectionname[] = TEXT("PMDPlayer"), profilename[] = TEXT(".\\pmdplay.ini"),
varstring_notifycount[] = TEXT("NotifyCount"), varstring_bufferblocktime[] = TEXT("BufferBlockTimeMS");

class XAPlayer
{
public:
	void Init(int nChannel, int sampleRate, int bytesPerVar, int onebufbytes);
	void Release();
	void *LockBuffer(DWORD length);
	void UnlockBuffer();
	void Play();
	void Stop();
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

class PMDPlayer
{
public:
	//nChannel: 通道数，1为单声道，2为立体声
	//sampleRate: 采样率（每秒多少采样）
	//bytesPerVar: 一个采样点的一个通道占多少字节
	//buffer_time_ms: 毫秒，表示buffer表示多长时间的数据
	//构造调用
	void Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms);
	//析构调用
	void Release();
	//从文件加载，成功返回0
	int LoadFromFile(const char* filepath);
	//从内存加载，成功返回0
	int LoadFromMemory(uchar* pdata, int length);
	//从文件加载节奏声音
	bool LoadRhythmFromDirectory(char* dir);
	//从内存加载节奏声音
	bool LoadRhythmFromMemory(char* bd, char* sd, char* top, char* hh, char* tom, char* rim);
	//转换PMD格式文件到WAV
	bool Convert(char *srcfile, char *outfile, int loops, int fadetime);
	//播放，没有加载时返回-1，否则为0
	int Play();
	//暂停，淡出和已处于暂停状态时返回-1，否则为0
	int Pause();
	//设置变频的播放速度控制，1为原速，成功返回0，否则为DSound错误码
	HRESULT SetPlaybackSpeed(float);
	//获取当前的播放速度倍率
	float GetPlaybackSpeed();
	//设置音量（0～100(PMDPLAYER_MAX_VOLUME)）
	void SetVolume(int);
	//获取音量（0～100(PMDPLAYER_MAX_VOLUME)）
	int GetVolume();
	//停止并释放
	void Unload();
	//淡出停止，未播放时返回-1
	int FadeoutAndStop(int time_ms);
	//获取按键状态
	const int *GetKeysState();
	//获取文档元数据中的信息，成功返回0，如果没有加载文件调用该函数时返回-1
	//AL	定義
	//-2	#PPZFile(1,2共用。MMLに記述した文字列そのまま)
	//-1	#PPSFile
	// 0	#PCMFile または #PPCFile
	// 1	#Title
	// 2	#Composer
	// 3	#Arranger
	// 4	#Memo(1個目)
	// 5	#Memo(2個目)
	//……	……
	int GetNotes(char* outstr, int al);
	//获取总时长
	unsigned GetLengthInMs();
	//获取当前位置
	unsigned GetPositionInMs();
	//获取循环长度
	unsigned GetLoopLengthInMs();
	//获取循环次数
	int GetLoopedTimes();
	//获取速度
	int GetTempo();
	//获取当前Tick
	int GetPositionInCount();
	//获取小节长度
	int GetXiaojieLength();
	enum PlayerStatus { nofile, paused, playing, fadingout, fadedout };
	//获取播放状态
	PlayerStatus GetPlayerStatus();
	const int *GetKeyVoice();
	const int *GetKeyVolume();

	//多线程调用
	static void _Subthread_Playback(PMDPlayer* param);
	//多线程播放函数
	void _LoopPlayback();
protected:
	void OnPlay();
	void OnFadingOut();
private:
	unsigned length_in_ms, loop_in_ms;
	PlayerStatus playerstatus;
	unsigned fadingout_end_time_sec;
	int bytesof_soundbuffer;//字节长度

	int m_channels;
	int m_bytesPerVar;
	int m_sampleRate;
	int keyState[NumOfAllPart];//音高
	int voiceState[NumOfAllPart];//音色
	int volumeState[NumOfAllPart];//音量
	float playbackspeed;
	uchar* pSourceData;
	int lengthSourceData;
	XAPlayer x;
	std::thread tSubPlayback;
};

