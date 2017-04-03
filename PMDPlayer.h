#pragma once
//https://github.com/mistydemeo/pmdmini
#include <xaudio2.h>
#include "pmdwin\pmdwinimport.h"

//https://github.com/lxfly2000/XAPlayer
class XAPlayer
{
public:
	XAPlayer(int nChannel, int sampleRate, int bytesPerVar);
	~XAPlayer();
	void Init(int nChannel, int sampleRate, int bytesPerVar);
	void Release();
	void Play(BYTE* buf, int length);
	int GetQueuedBuffersNum();
private:
	IXAudio2*xAudio2Engine;
	IXAudio2MasteringVoice* masterVoice;
	IXAudio2SourceVoice* sourceVoice;
	XAUDIO2_BUFFER xbuffer;
	XAUDIO2_VOICE_STATE state;
};

class PMDPlayer
{
public:
	//nChannel: 通道数，1为单声道，2为立体声
	//sampleRate: 采样率（每秒多少采样）
	//bytesPerVar: 一个采样点的一个通道占多少字节
	//buffer_time_ms: 毫秒，表示buffer表示多长时间的数据
	PMDPlayer(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms);
	~PMDPlayer();
	//构造调用
	void Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms);
	//析构调用
	void Release();
	//从文件加载，成功返回0
	int LoadFromFile(const char* filepath);
	//从内存加载，成功返回0
	int LoadFromMemory(uchar* pdata, int length);
	//播放，没有加载时返回-1，否则为0
	int Play();
	//暂停，淡出和已处于暂停状态时返回-1，否则为0
	int Pause();
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
	enum PlayerStatus { nofile, paused, playing, fadingout };
	//获取播放状态
	PlayerStatus GetPlayerStatus();

	//多线程调用
	static void _Subthread_Playback(void* param);
	//多线程播放函数
	void _LoopPlayback();
protected:
	void OnPlay();
	void OnFadingOut();
private:
	unsigned length_in_ms, loop_in_ms;
	PlayerStatus playerstatus;
	unsigned fadingout_end_time_sec;
	short* soundbuffer;//PMD_Renderer那个函数用的类型是short我表示难以理解……
	int bytesof_soundbuffer;//字节长度

	int m_channels;
	int m_bytesPerVar;
	int keyState[NumOfAllPart];
	uchar* pSourceData;
	int lengthSourceData;
	XAPlayer x;
	static bool subthread_on;
};

