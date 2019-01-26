#pragma once
//https://github.com/mistydemeo/pmdmini
#include <thread>

#define SECTION_NAME	"PMDPlayer"
#define PROFILE_NAME	".\\pmdplay.ini"

#define PMDPLAYER_CHANNELS_NUM	24
#define PMDPLAYER_MAX_VOLUME	100

#ifdef _DEBUG
#define C(e) if(e)\
if(MessageBox(NULL,__FILE__ L":" _STRINGIZE(__LINE__) "\n" __FUNCTION__ "\n" _STRINGIZE(e),NULL,MB_OKCANCEL)==IDOK)\
DebugBreak()
#else
#define C(e) e
#endif

class PMDPlayer
{
public:
	//nChannel: 通道数，1为单声道，2为立体声
	//sampleRate: 采样率（每秒多少采样）
	//bytesPerVar: 一个采样点的一个通道占多少字节
	//buffer_time_ms: 毫秒，表示buffer表示多长时间的数据，如果为0表示使用默认值
	//构造调用，成功返回0，尾部追加
	virtual int Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms);
	//析构调用，成功返回0，尾部追加
	virtual int Release();
	//从文件加载，成功返回0
	int LoadFromFile(const char* filepath);
	//从内存加载，成功返回0
	int LoadFromMemory(unsigned char* pdata, int length);
	//从文件加载节奏声音
	bool LoadRhythmFromDirectory(char* dir);
	//从内存加载节奏声音
	bool LoadRhythmFromMemory(char* bd, char* sd, char* top, char* hh, char* tom, char* rim);
	//转换PMD格式文件到WAV
	bool Convert(char *srcfile, char *outfile, int loops, int fadetime, bool splittracks);
	//播放，没有加载时返回-1，否则为0，头部追加
	virtual int Play();
	//暂停，淡出和已处于暂停状态时返回-1，否则为0，尾部追加
	virtual int Pause();
	//设置变频的播放速度控制，1为原速，成功返回0，否则为DSound错误码，直接覆盖
	virtual int SetPlaybackSpeed(float);
	//获取当前的播放速度倍率，直接覆盖
	virtual float GetPlaybackSpeed();
	//设置音量（0～100(PMDPLAYER_MAX_VOLUME)），直接覆盖
	virtual void SetVolume(int);
	//获取音量（0～100(PMDPLAYER_MAX_VOLUME)），直接覆盖
	virtual int GetVolume();
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
	//设置播放位置
	void SetPositionInMs(int);
	//开启/关闭通道
	void SetChannelOn(int, bool);
	//开启/关闭SSG节奏声音
	void SetSSGEffectOn(bool);
	bool GetSSGEffectOn();
	//开启/关闭YM2608节奏声音
	void SetRhythmOn(bool);
	//停止播放（播放位置被重置到开始），尾部追加
	virtual int Stop();

	//多线程调用
	static void _Subthread_Playback(PMDPlayer* param);
	//多线程播放函数
	void _LoopPlayback();
protected:
	//头部追加
	virtual void OnPlay();
	void OnFadingOut();
	int bytesof_soundbuffer;//字节长度

	float playbackspeed;
	int m_channels;
	int m_bytesPerVar;
private:
	unsigned length_in_ms, loop_in_ms;
	PlayerStatus playerstatus;
	unsigned fadingout_end_time_sec;

	int m_sampleRate;
	int keyState[PMDPLAYER_CHANNELS_NUM];//音高
	int voiceState[PMDPLAYER_CHANNELS_NUM];//音色
	int volumeState[PMDPLAYER_CHANNELS_NUM];//音量
	unsigned char* pSourceData;
	int lengthSourceData;
	std::thread tSubPlayback;
};

